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

	bool begin() const noexcept;
	bool end() const noexcept;

	std::string lastError() const { return m_lastError.lastError(); }
	int lastErrorCode() const { return m_lastErrorCode; }
	int lastErrorCodeExt() const { return m_lastErrorCodeExt; }

protected:
	SQLConn() : m_flags(0), m_lastErrorCode(0), m_lastErrorCodeExt(0) {}

	enum TableFlags : unsigned {
		TABLE_TEMPORARY = 1u << 0,
	};

	struct TableEntry {
		std::string name;
		std::vector<std::string> columns;
		unsigned flags = 0u;
	};

	using Tables = std::vector<TableEntry>;
	using Indices = std::vector<std::pair<std::string, std::string>>;
	using Triggers = Indices;
	using Views = Indices;
	using Statements = std::vector<std::pair<std::reference_wrapper<SQLStmtHolder>, std::string>>;

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
	bool prepareStatements(const Statements &stmts) const noexcept;

	bool bind(const SQLStmtHolder &ins, const std::string &key,
		  const BindVal &val, bool transient = false) const noexcept;
	bool bind(const SQLStmtHolder &ins, const Binding &binding,
		  bool transient = false) const noexcept;
	bool step(const SQLStmtHolder &ins, uint64_t *affected = nullptr) const noexcept;
	bool insert(const SQLStmtHolder &ins, const Binding &binding,
		    uint64_t *affected = nullptr) const noexcept;

	std::optional<SQLConn::SelectResult>
	select(const SQLStmtHolder &sel, const Binding &binding,
	       const ColumnTypes &columns) const noexcept;

	SlHelpers::LastError &setError(int ret, const std::string_view &error,
				       bool errmsg = false) const;

	SQLHolder sqlHolder;
	unsigned int m_flags;
	mutable SlHelpers::LastError m_lastError;
	mutable int m_lastErrorCode;
	mutable int m_lastErrorCodeExt;
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
