// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLSQLITE_SQLCONN_H
#define SLSQLITE_SQLCONN_H

#include <filesystem>
#include <string>
#include <typeindex>
#include <variant>
#include <vector>

#include "../helpers/LastError.h"
#include "SQLiteSmart.h"

namespace SlSqlite {

enum open_flags {
	CREATE		= 1 << 0,
	NO_FOREIGN_KEY	= 1 << 1,
};

class SQLConn {
public:
	int open(const std::filesystem::path &dbFile, unsigned int flags = 0)
	{
		if (openDB(dbFile, flags) ||
				createDB() ||
				prepDB())
			return -1;

		return 0;
	}

	int openDB(const std::filesystem::path &dbFile, unsigned int flags = 0) noexcept;
	virtual int createDB() { return 0; }
	virtual int prepDB() { return 0; }

	int begin();
	int end();

	std::string lastError() const { return m_lastError.lastError(); }

protected:
	SQLConn() {}

	using Tables = std::vector<std::pair<std::string, std::vector<std::string>>>;
	using Indices = std::vector<std::pair<std::string, std::string>>;
	using Views = Indices;

	using BindVal = std::variant<std::monostate, int, std::string>;
	using Binding = std::vector<std::pair<std::string, BindVal>>;
	using ColumnTypes = std::vector<std::type_index>;
	using Column = std::variant<int, std::string>;
	using Row = std::vector<Column>;
	using SelectResult = std::vector<Row>;

	int createTables(const Tables &tables) const noexcept;
	int createIndices(const Indices &indices) const noexcept;
	int createViews(const Views &views) const noexcept;

	int prepareStatement(const std::string &sql, SQLStmtHolder &stmt) const noexcept;

	int bind(const SQLStmtHolder &ins, const std::string &key, const BindVal &val) const noexcept;
	int bind(const SQLStmtHolder &ins, const Binding &binding) const noexcept;
	int insert(const SQLStmtHolder &ins, const Binding &binding) const noexcept;
	int select(const SQLStmtHolder &sel, const Binding &binding, const ColumnTypes &columns,
		   SelectResult &result) const noexcept;

	SQLHolder sqlHolder;
	mutable SlHelpers::LastError m_lastError;
};

}

#endif
