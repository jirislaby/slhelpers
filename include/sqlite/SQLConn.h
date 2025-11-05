// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLSQLITE_SQLCONN_H
#define SLSQLITE_SQLCONN_H

#include <filesystem>
#include <optional>
#include <string>
#include <typeindex>
#include <variant>
#include <vector>

#include "../helpers/LastError.h"
#include "SQLiteSmart.h"

namespace SlSqlite {

enum OpenFlags : unsigned {
	CREATE				= 1 << 0,
	NO_FOREIGN_KEY			= 1 << 1,
	ERROR_ON_UNIQUE_CONSTRAINT	= 1 << 2,
};

class SQLConn {
	friend class Select;
public:
	bool open(const std::filesystem::path &dbFile, unsigned int flags = 0)
	{
		if (!openDB(dbFile, flags) ||
				!createDB() ||
				!prepDB())
			return false;

		return true;
	}

	bool openDB(const std::filesystem::path &dbFile, unsigned int flags = 0) noexcept;
	virtual bool createDB() { return true; }
	virtual bool prepDB() { return true; }

	bool begin();
	bool end();

	std::string lastError() const { return m_lastError.lastError(); }

protected:
	SQLConn() : m_flags(0) {}

	using Tables = std::vector<std::pair<std::string, std::vector<std::string>>>;
	using Indices = std::vector<std::pair<std::string, std::string>>;
	using Triggers = Indices;
	using Views = Indices;

	using BindVal = std::variant<std::monostate, int, std::string, std::string_view>;
	using Binding = std::vector<std::pair<std::string, BindVal>>;
	using ColumnTypes = std::vector<std::type_index>;
	using Column = std::variant<int, std::string>;
	using Row = std::vector<Column>;
	using SelectResult = std::vector<Row>;

	bool createTables(const Tables &tables) const noexcept;
	bool createIndices(const Indices &indices) const noexcept;
	bool createTriggers(const Triggers &triggers) const noexcept;
	bool createViews(const Views &views) const noexcept;

	bool prepareStatement(const std::string &sql, SQLStmtHolder &stmt) const noexcept;

	bool bind(const SQLStmtHolder &ins, const std::string &key,
		  const BindVal &val) const noexcept;
	bool bind(const SQLStmtHolder &ins, const Binding &binding) const noexcept;
	bool insert(const SQLStmtHolder &ins, const Binding &binding,
		    uint64_t *affected = nullptr) const noexcept;

	std::optional<SQLConn::SelectResult>
	select(const SQLStmtHolder &sel, const Binding &binding,
	       const ColumnTypes &columns) const noexcept;

	SQLHolder sqlHolder;
	unsigned int m_flags;
	mutable SlHelpers::LastError m_lastError;
private:
	static int busyHandler(void *, int count);
};

class Select {
public:
	Select() = delete;
	Select(const SQLConn &sqlConn) : m_sqlConn(sqlConn) {}

	bool prepare(const std::string &sql, const SQLConn::ColumnTypes &columns) noexcept {
		m_resultTypes = columns;
		return m_sqlConn.prepareStatement(sql, m_select);
	}

	std::optional<SQLConn::SelectResult>
	select(const SQLConn::Binding &binding) const noexcept {
		return m_sqlConn.select(m_select, binding, m_resultTypes);
	}
private:
	const SQLConn &m_sqlConn;
	SQLStmtHolder m_select;
	SQLConn::ColumnTypes m_resultTypes;
};

}

#endif
