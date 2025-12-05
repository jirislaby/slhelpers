// SPDX-License-Identifier: GPL-2.0-only

#include <chrono>
#include <optional>
#include <sqlite3.h>
#include <thread>
#include <typeindex>

#include "helpers/String.h"
#include "sqlite/SQLConn.h"

#include "PtrStore.h"

using namespace SlSqlite;

int SQLConn::busyHandler(void *, int count)
{
	static const auto WAIT_INTVAL = std::chrono::milliseconds(20);
	static const auto WAIT_TIMEOUT = std::chrono::seconds(10) / WAIT_INTVAL;

	if (count >= WAIT_TIMEOUT)
		return 0;

	std::this_thread::sleep_for(WAIT_INTVAL);

	return 1;
}

bool SQLConn::openDB(const std::filesystem::path &dbFile, unsigned int flags) noexcept
{
	sqlite3 *sql;
	int openFlags = SQLITE_OPEN_READWRITE;

	m_flags = flags;

	if (flags & OpenFlags::CREATE)
		openFlags |= SQLITE_OPEN_CREATE;

	auto ret = sqlite3_open_v2(dbFile.c_str(), &sql, openFlags, nullptr);
	sqlHolder.reset(sql);
	if (ret != SQLITE_OK) {
		setError(ret, "db open failed");
		return false;
	}

	if (!(flags & OpenFlags::NO_FOREIGN_KEY)) {
		CharPtrStore err;
		ret = sqlite3_exec(sqlHolder, "PRAGMA foreign_keys = ON;", nullptr, nullptr,
				   err.ptr());
		if (ret != SQLITE_OK) {
			setError(ret, "db PRAGMA failed") << " -> " << err;
			return false;
		}
	}

	ret = sqlite3_busy_handler(sqlHolder, busyHandler, nullptr);
	if (ret != SQLITE_OK) {
		setError(ret, "db busy_handler failed");
		return false;
	}

	return true;
}

bool SQLConn::attach(const std::filesystem::path &dbFile,
		     const std::string_view &dbName) const noexcept
{
	SQLStmtHolder attach;
	return prepareStatement("ATTACH DATABASE :db AS :dbName", attach) &&
			insert(attach, {
				       { ":db", dbFile.string() },
				       { ":dbName", dbName }
			       });
}

bool SQLConn::createTables(const Tables &tables) const noexcept
{
	for (const auto &c: tables) {
		std::ostringstream ss;
		ss << "CREATE";
		if (c.flags & TABLE_TEMPORARY)
			ss << " TEMPORARY";
		ss << " TABLE IF NOT EXISTS " << c.name << '(';
		SlHelpers::String::join(ss, c.columns);
		ss << ") STRICT;";
		const auto expr = ss.str();
		CharPtrStore err;
		const auto ret = sqlite3_exec(sqlHolder, expr.c_str(), nullptr, nullptr, err.ptr());
		if (ret != SQLITE_OK) {
			setError(ret, "db CREATE TABLE failed") << " -> " << err << "\n\t" << expr;
			return false;
		}
	}

	return true;
}

bool SQLConn::createIndices(const Indices &indices) const noexcept
{
	for (const auto &c: indices) {
		std::string s("CREATE INDEX IF NOT EXISTS ");
		s.append(c.first).append(" ON ").append(c.second);
		CharPtrStore err;
		const auto ret = sqlite3_exec(sqlHolder, s.c_str(), nullptr, nullptr, err.ptr());
		if (ret != SQLITE_OK) {
			setError(ret, "db CREATE INDEX failed") << " -> " << err << "\n\t" << s;
			return false;
		}
	}

	return true;
}

bool SQLConn::createTriggers(const Triggers &triggers) const noexcept
{
	for (const auto &c: triggers) {
		std::string s("CREATE TRIGGER IF NOT EXISTS ");
		s.append(c.first).append(" FOR EACH ROW BEGIN ").append(c.second).append("; END;");
		CharPtrStore err;
		const auto ret = sqlite3_exec(sqlHolder, s.c_str(), nullptr, nullptr, err.ptr());
		if (ret != SQLITE_OK) {
			setError(ret, "db CREATE TRIGGER failed") << " -> " << err << "\n\t" << s;
			return false;
		}
	}

	return true;
}

bool SQLConn::createViews(const Views &views) const noexcept
{
	for (const auto &c: views) {
		std::string s("CREATE VIEW IF NOT EXISTS ");
		s.append(c.first).append(" AS ").append(c.second);
		CharPtrStore err;
		const auto ret = sqlite3_exec(sqlHolder, s.c_str(), nullptr, nullptr, err.ptr());
		if (ret != SQLITE_OK) {
			setError(ret, "db CREATE VIEW failed") << " -> " << err << "\n\t" << s;
			return false;
		}
	}

	return true;
}

bool SQLConn::prepareStatement(const std::string &sql, SQLStmtHolder &stmt) const noexcept
{
	sqlite3_stmt *SQLStmt;
	auto ret = sqlite3_prepare_v2(sqlHolder, sql.c_str(), -1, &SQLStmt, nullptr);
	if (ret != SQLITE_OK) {
		setError(ret, "db prepare failed", true) << "\n\t" << sql;
		return false;
	}

	stmt.reset(SQLStmt);

	return true;
}

bool SQLConn::prepareStatements(const Statements &stmts) const noexcept
{
	for (const auto &e: stmts)
		if (!prepareStatement(e.second, e.first))
			return false;

	return true;
}

bool SQLConn::prepareSelects(const Selects &sels) const noexcept
{
	for (const auto &e: sels)
		if (!e.ref.get().prepare(e.stmt, e.columnTypes))
			return false;

	return true;
}

bool SQLConn::begin(TransactionType type) const noexcept
{
	std::string stmt("BEGIN ");
	switch (type) {
	default:
		stmt.append("DEFERRED");
		break;
	case TransactionType::IMMEDIATE:
		stmt.append("IMMEDIATE");
		break;
	case TransactionType::EXCLUSIVE:
		stmt.append("EXCLUSIVE");
		break;
	}
	stmt.append(";");

	CharPtrStore err;
	const auto ret = sqlite3_exec(sqlHolder, stmt.c_str(), nullptr, nullptr, err.ptr());
	if (ret != SQLITE_OK) {
		setError(ret, "db BEGIN failed") << " -> " << err;
		return false;
	}

	return true;
}

bool SQLConn::end() const noexcept
{
	CharPtrStore err;
	const auto ret = sqlite3_exec(sqlHolder, "END;", nullptr, nullptr, err.ptr());
	if (ret != SQLITE_OK) {
		setError(ret, "db END failed") << " -> " << err;
		return false;
	}

	return true;
}

bool SQLConn::bind(const SQLStmtHolder &ins, const std::string &key,
		   const BindVal &val, bool transient) const noexcept
{
	auto bindIdx = sqlite3_bind_parameter_index(ins, key.c_str());
	if (!bindIdx) {
		m_lastError.reset() << "no index found for key=" << key;
		return false;
	}

	auto flag = transient ? SQLITE_TRANSIENT : SQLITE_STATIC;
	int ret;
	std::string valDesc { "null" };
	if (std::holds_alternative<int>(val)) {
		const auto &i = std::get<int>(val);
		valDesc = std::to_string(i);
		ret = sqlite3_bind_int(ins, bindIdx, i);
	} else if (std::holds_alternative<unsigned>(val)) {
		const auto &i = std::get<unsigned>(val);
		valDesc = std::to_string(i);
		ret = sqlite3_bind_int(ins, bindIdx, i);
	} else if (std::holds_alternative<std::string>(val)) {
		const auto &text = std::get<std::string>(val);
		valDesc = text;
		ret = sqlite3_bind_text(ins, bindIdx, text.data(), text.length(), flag);
	} else if (std::holds_alternative<std::string_view>(val)) {
		const auto &text = std::get<std::string_view>(val);
		valDesc = text;
		ret = sqlite3_bind_text(ins, bindIdx, text.data(), text.length(), flag);
	} else { /* std::monostate */
		ret = sqlite3_bind_null(ins, bindIdx);
	}

	if (ret != SQLITE_OK) {
		setError(ret, "db bind failed") << "\n\tkey=\"" << key << "\" val=\"" <<
						   valDesc << '"';
		return false;
	}

	return true;
}

bool SQLConn::bind(const SQLStmtHolder &ins, const Binding &binding, bool transient) const noexcept
{
	for (const auto &b : binding)
		if (!bind(ins, b.first, b.second, transient))
			return false;

	return true;
}

bool SQLConn::step(const SQLStmtHolder &ins, uint64_t *affected) const noexcept
{
	auto ret = sqlite3_step(ins);
	if (ret == SQLITE_DONE) {
		if (affected)
			*affected = sqlite3_changes64(sqlHolder);
		return true;
	}

	setError(ret, "db step (INSERT) failed", true);

	if (m_lastErrorCodeExt == SQLITE_CONSTRAINT_UNIQUE &&
			!(m_flags & OpenFlags::ERROR_ON_UNIQUE_CONSTRAINT)) {
		if (affected)
			*affected = 0;
		return true;
	}

	return false;
}

void SQLConn::dumpBinding(const Binding &binding) const noexcept
{
	for (const auto &b : binding) {
		m_lastError << '\t' << b.first << '=';
		if (std::holds_alternative<int>(b.second))
			m_lastError << "I:" << std::get<int>(b.second);
		if (std::holds_alternative<unsigned>(b.second))
			m_lastError << "U:" << std::get<unsigned>(b.second);
		else if (std::holds_alternative<std::string>(b.second))
			m_lastError << "T:" << std::get<std::string>(b.second);
		else if (std::holds_alternative<std::string_view>(b.second))
			m_lastError << "T:" << std::get<std::string_view>(b.second);
		else
			m_lastError << "NULL";
	}
}

bool SQLConn::insert(const SQLStmtHolder &ins, const Binding &binding,
		     uint64_t *affected) const noexcept
{
	SQLStmtResetter insResetter(sqlHolder, ins);

	if (!bind(ins, binding))
		return false;

	if (!step(ins, affected)) {
		dumpBinding(binding);
		return false;
	}

	auto ret = insResetter.reset();
	if (ret != SQLITE_OK) {
		setError(ret, "db stmt reset (INSERT) failed", true);
		dumpBinding(binding);
		return false;
	}

	return true;
}

std::optional<SQLConn::SelectResult>
SQLConn::select(const SQLStmtHolder &sel, const Binding &binding,
		const ColumnTypes &columns) const noexcept
{
	SQLStmtResetter selResetter(sqlHolder, sel);
	int ret;

	if (!bind(sel, binding))
		return std::nullopt;

	SQLConn::SelectResult result;
	while (true) {
		ret = sqlite3_step(sel);
		if (ret == SQLITE_DONE)
			return result;
		if (ret != SQLITE_ROW) {
			setError(ret, "db step (SELECT) failed", true);
			return std::nullopt;
		}

		Row row;

		for (unsigned i = 0; i < columns.size(); ++i) {
			Column col;
			if (columns[i] == typeid(int))
				col = sqlite3_column_int(sel, i);
			else if (columns[i] == typeid(std::string))
				col = Column{reinterpret_cast<const char *>(sqlite3_column_text(sel, i))};
			row.push_back(std::move(col));
		}
		result.push_back(std::move(row));
	}
}

SlHelpers::LastError &SQLConn::setError(int ret, const std::string_view &error, bool errmsg) const
{
	m_lastErrorCode = sqlite3_errcode(sqlHolder);
	m_lastErrorCodeExt = sqlite3_extended_errcode(sqlHolder);
	m_lastError.reset() << error << ": " << sqlite3_errstr(ret);
	if (errmsg)
		m_lastError << " -> " << sqlite3_errmsg(sqlHolder);

	return m_lastError;
}
