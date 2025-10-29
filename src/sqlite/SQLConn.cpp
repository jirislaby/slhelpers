// SPDX-License-Identifier: GPL-2.0-only

#include <chrono>
#include <sqlite3.h>
#include <thread>
#include <typeindex>
#include <vector>

#include "sqlite/SQLConn.h"

using namespace SlSqlite;

static int busy_handler(void *, int count)
{
	static const auto WAIT_INTVAL = std::chrono::milliseconds(20);
	static const auto WAIT_TIMEOUT = std::chrono::seconds(10) / WAIT_INTVAL;

	if (count >= WAIT_TIMEOUT)
		return 0;

	std::this_thread::sleep_for(WAIT_INTVAL);

	return 1;
}

static void joinVec(std::ostringstream &ss, const std::vector<std::string> &vec,
		    const std::string &sep = ", ")
{
	for (auto i = vec.begin(), end = vec.end(); i != end; ++i) {
		ss << *i;
		if (i != end - 1)
			ss << sep;
	}
}

int SQLConn::openDB(const std::filesystem::path &dbFile, unsigned int flags) noexcept
{
	sqlite3 *sql;
	int openFlags = SQLITE_OPEN_READWRITE;
	int ret;

	m_flags = flags;

	if (flags & OpenFlags::CREATE)
		openFlags |= SQLITE_OPEN_CREATE;

	ret = sqlite3_open_v2(dbFile.c_str(), &sql, openFlags, NULL);
	sqlHolder.reset(sql);
	if (ret != SQLITE_OK) {
		m_lastError.reset() << "db open failed: " << sqlite3_errstr(ret);
		return -1;
	}

	if (!(flags & OpenFlags::NO_FOREIGN_KEY)) {
		char *err;
		ret = sqlite3_exec(sqlHolder, "PRAGMA foreign_keys = ON;", NULL, NULL, &err);
		if (ret != SQLITE_OK) {
			m_lastError.reset() << "db PRAGMA failed (" << __LINE__ << "): " <<
					       sqlite3_errstr(ret) << " -> " << err;
			sqlite3_free(err);
			return -1;
		}
	}

	ret = sqlite3_busy_handler(sqlHolder, busy_handler, nullptr);
	if (ret != SQLITE_OK) {
		m_lastError.reset() << "db busy_handler failed (" << __LINE__ << "): " <<
				       sqlite3_errstr(ret);
		return -1;
	}

	return 0;
}

int SQLConn::createTables(const Tables &tables) const noexcept
{
	char *err;
	int ret;

	for (const auto &c: tables) {
		std::ostringstream ss;
		ss << "CREATE TABLE IF NOT EXISTS " << c.first << '(';
		joinVec(ss, c.second);
		ss << ") STRICT;";
		const auto expr = ss.str();
		ret = sqlite3_exec(sqlHolder, expr.c_str(), NULL, NULL, &err);
		if (ret != SQLITE_OK) {
			m_lastError.reset() << "db CREATE failed (" << __LINE__ << "): " <<
					       sqlite3_errstr(ret) << " -> " << err << "\n\t" <<
					       expr;
			sqlite3_free(err);
			return -1;
		}
	}

	return 0;
}

int SQLConn::createIndices(const Indices &indices) const noexcept
{
	char *err;
	int ret;

	for (const auto &c: indices) {
		std::string s("CREATE INDEX IF NOT EXISTS ");
		s.append(c.first).append(" ON ").append(c.second);
		ret = sqlite3_exec(sqlHolder, s.c_str(), NULL, NULL, &err);
		if (ret != SQLITE_OK) {
			m_lastError.reset() << "db CREATE failed (" << __LINE__ << "): " <<
					       sqlite3_errstr(ret) << " -> " << err << "\n\t" << s;
			sqlite3_free(err);
			return -1;
		}
	}

	return 0;
}

int SQLConn::createViews(const Views &views) const noexcept
{
	char *err;
	int ret;

	for (const auto &c: views) {
		std::string s("CREATE VIEW IF NOT EXISTS ");
		s.append(c.first).append(" AS ").append(c.second);
		ret = sqlite3_exec(sqlHolder, s.c_str(), NULL, NULL, &err);
		if (ret != SQLITE_OK) {
			m_lastError.reset() << "db CREATE failed (" << __LINE__ << "): " <<
					       sqlite3_errstr(ret) << " -> " << err << "\n\t" << s;
			sqlite3_free(err);
			return -1;
		}
	}

	return 0;
}

int SQLConn::prepareStatement(const std::string &sql, SQLStmtHolder &stmt) const noexcept
{
	sqlite3_stmt *SQLStmt;
	int ret = sqlite3_prepare_v2(sqlHolder, sql.c_str(), -1, &SQLStmt, NULL);
	if (ret != SQLITE_OK) {
		m_lastError.reset() << "db prepare failed (" << __LINE__ << "): " <<
				       sqlite3_errstr(ret) << " -> " <<
				       sqlite3_errmsg(sqlHolder);
		return -1;
	}

	stmt.reset(SQLStmt);

	return 0;
}

int SQLConn::begin()
{
	char *err;
	int ret = sqlite3_exec(sqlHolder, "BEGIN;", NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		m_lastError.reset() << "db BEGIN failed (" << __LINE__ << "): " <<
				       sqlite3_errstr(ret) << " -> " << err;
		sqlite3_free(err);
		return -1;
	}

	return 0;
}

int SQLConn::end()
{
	char *err;
	int ret = sqlite3_exec(sqlHolder, "END;", NULL, NULL, &err);
	if (ret != SQLITE_OK) {
		m_lastError.reset() << "db END failed (" << __LINE__ << "): " <<
				       sqlite3_errstr(ret) << " -> " << err;
		sqlite3_free(err);
		return -1;
	}

	return 0;
}

int SQLConn::bind(const SQLStmtHolder &ins, const std::string &key,
		  const BindVal &val) const noexcept
{
	auto bindIdx = sqlite3_bind_parameter_index(ins, key.c_str());
	if (!bindIdx) {
		m_lastError.reset() << "no index found for key=" << key;
		return -1;
	}

	int ret;
	std::string valDesc { "null" };
	if (std::holds_alternative<int>(val)) {
		const auto &i = std::get<int>(val);
		valDesc = std::to_string(i);
		ret = sqlite3_bind_int(ins, bindIdx, i);
	} else if (std::holds_alternative<std::string>(val)) {
		const auto &text = std::get<std::string>(val);
		valDesc = text;
		ret = sqlite3_bind_text(ins, bindIdx, text.data(), text.length(), SQLITE_STATIC);
	} else { /* std::monostate */
		ret = sqlite3_bind_null(ins, bindIdx);
	}

	if (ret != SQLITE_OK) {
		m_lastError.reset() << "db bind failed (" << __LINE__ << " key=\"" << key <<
				       "\" val=\"" << valDesc << "\"): " <<
				       sqlite3_errstr(ret) << " -> " <<
				       sqlite3_errmsg(sqlHolder);
		return -1;
	}

	return 0;
}

int SQLConn::bind(const SQLStmtHolder &ins, const Binding &binding) const noexcept
{
	for (const auto &b : binding)
		if (bind(ins, b.first, b.second))
			return -1;

	return 0;
}

int SQLConn::insert(const SQLStmtHolder &ins, const Binding &binding,
		    uint64_t *affected) const noexcept
{
	SQLStmtResetter insResetter(sqlHolder, ins);
	int ret;

	if (bind(ins, binding))
		return -1;

	ret = sqlite3_step(ins);
	if (ret == SQLITE_DONE) {
		if (affected)
			*affected = sqlite3_changes64(sqlHolder);
		return 0;
	}

	if (sqlite3_extended_errcode(sqlHolder) == SQLITE_CONSTRAINT_UNIQUE &&
			!(m_flags & OpenFlags::ERROR_ON_UNIQUE_CONSTRAINT)) {
		if (affected)
			*affected = 0;
		return 0;
	}

	m_lastError.reset() << "db step (INSERT) failed (" << __LINE__ << "): " <<
			       sqlite3_errstr(ret) << " -> " <<
			       sqlite3_errmsg(sqlHolder);
	for (const auto &b : binding) {
		m_lastError << '\t' << b.first << '=';
		if (std::holds_alternative<int>(b.second))
			m_lastError << "I:" << std::get<int>(b.second);
		else if (std::holds_alternative<std::string>(b.second))
			m_lastError << "T:" << std::get<std::string>(b.second);
		else
			m_lastError << "NULL";
	}
	return -1;
}

int SQLConn::select(const SQLStmtHolder &sel, const Binding &binding, const ColumnTypes &columns,
		    SelectResult &result) const noexcept
{
	SQLStmtResetter selResetter(sqlHolder, sel);
	int ret;

	if (bind(sel, binding))
		return -1;

	while (true) {
		ret = sqlite3_step(sel);
		if (ret == SQLITE_DONE)
			return 0;
		if (ret != SQLITE_ROW) {
			m_lastError.reset() << "db step (SELECT) failed (" << __LINE__ << "): " <<
					       sqlite3_errstr(ret) << " -> " <<
					       sqlite3_errmsg(sqlHolder);
			return -1;
		}

		Row row;

		for (unsigned i = 0; i < columns.size(); ++i) {
			Column col;
			if (columns[i] == typeid(int))
				col = sqlite3_column_int(sel, i);
			else if (columns[i] == typeid(std::string))
				col = Column{(char *)sqlite3_column_text(sel, i)};
			row.push_back(std::move(col));
		}
		result.push_back(std::move(row));
	}

	return 0;
}
