// SPDX-License-Identifier: GPL-2.0-only

#include <chrono>
#include <optional>
#include <sqlite3.h>
#include <thread>
#include <typeindex>

#include "helpers/PtrStore.h"
#include "helpers/String.h"
#include "sqlite/SQLConn.h"

using namespace SlSqlite;

using CharPtrStore = SlHelpers::PtrStore<char, decltype([](void* p) { sqlite3_free(p); })>;

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

	if (!(flags & OpenFlags::NO_FOREIGN_KEY))
		if (!exec("PRAGMA foreign_keys = ON;", "db PRAGMA failed"))
			return false;

	ret = sqlite3_busy_handler(sqlHolder, busyHandler, nullptr);
	if (ret != SQLITE_OK) {
		setError(ret, "db busy_handler failed");
		return false;
	}

	return true;
}

bool SQLConn::attach(const std::filesystem::path &dbFile,
		     std::string_view dbName) const noexcept
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
		if (!exec(ss.str(), "db CREATE TABLE failed", true))
			return false;
	}

	return true;
}

bool SQLConn::createIndices(const Indices &indices) const noexcept
{
	for (const auto &c: indices) {
		std::string s("CREATE INDEX IF NOT EXISTS ");
		s.append(c.first).append(" ON ").append(c.second);
		if (!exec(s, "db CREATE TABLE failed", true))
			return false;
	}

	return true;
}

bool SQLConn::createTriggers(const Triggers &triggers) const noexcept
{
	for (const auto &c: triggers) {
		std::string s("CREATE TRIGGER IF NOT EXISTS ");
		s.append(c.first).append(" FOR EACH ROW BEGIN ").append(c.second).append("; END;");
		if (!exec(s, "db CREATE TRIGGER failed", true))
			return false;
	}

	return true;
}

bool SQLConn::createViews(const Views &views) const noexcept
{
	for (const auto &c: views) {
		std::string s("CREATE VIEW IF NOT EXISTS ");
		s.append(c.first).append(" AS ").append(c.second);
		if (!exec(s, "db CREATE VIEW failed", true))
			return false;
	}

	return true;
}

bool SQLConn::prepareStatement(std::string_view sql, SQLStmtHolder &stmt) const noexcept
{
	sqlite3_stmt *SQLStmt;
	auto ret = sqlite3_prepare_v2(sqlHolder, sql.data(), sql.size(), &SQLStmt, nullptr);
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

	return exec(stmt, "db BEGIN failed");
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

bool SQLConn::step(const SQLStmtHolder &ins, uint64_t *affected, bool *uniqueError) const noexcept
{
	auto ret = sqlite3_step(ins);
	if (ret == SQLITE_DONE) {
		if (affected)
			*affected = sqlite3_changes64(sqlHolder);
		if (uniqueError)
			*uniqueError = false;
		return true;
	}

	const auto uniqueErrorLoc = isUniqueConstraint(sqlite3_extended_errcode(sqlHolder));
	if (uniqueError)
		*uniqueError = uniqueErrorLoc;

	if (uniqueErrorLoc && !(m_flags & OpenFlags::ERROR_ON_UNIQUE_CONSTRAINT)) {
		if (affected)
			*affected = 0;
		return true;
	}

	setError(ret, "db step (INSERT) failed", true);

	return false;
}

bool SQLConn::exec(const std::string &SQL, std::string_view errorMsg,
		   bool includeSQL) const noexcept
{
	CharPtrStore err;
	auto ret = sqlite3_exec(sqlHolder, SQL.c_str(), nullptr, nullptr, err.ptr());
	if (ret != SQLITE_OK) {
		auto &thisErr = setError(ret, errorMsg) << " -> " << err;
		if (includeSQL)
			thisErr << "\n\t" << SQL;
		return false;
	}

	return true;
}

constexpr bool SQLConn::isUniqueConstraint(int sqlExtError) noexcept
{
	switch (sqlExtError) {
	case SQLITE_CONSTRAINT_PRIMARYKEY:
	case SQLITE_CONSTRAINT_ROWID:
	case SQLITE_CONSTRAINT_UNIQUE:
		return true;
	default:
		return false;
	}
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
	SQLStmtResetter insResetter(ins);

	if (!bind(ins, binding))
		return false;

	bool uniqueError;
	if (!step(ins, affected, &uniqueError)) {
		dumpBinding(binding);
		return false;
	}

	auto ret = insResetter.reset();
	if (!uniqueError && ret != SQLITE_OK) {
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
	SQLStmtResetter selResetter(sel);
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

SQLConn::LastError &SQLConn::setError(int ret, std::string_view error, bool errmsg) const
{
	m_lastError.reset() << error << ": " << sqlite3_errstr(ret);
	if (errmsg)
		m_lastError << " -> " << sqlite3_errmsg(sqlHolder);

	m_lastError.set<0>(sqlite3_errcode(sqlHolder));
	m_lastError.set<1>(sqlite3_extended_errcode(sqlHolder));

	return m_lastError;
}
