// SPDX-License-Identifier: GPL-2.0-only

#pragma once

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

enum struct TransactionType {
	DEFERRED,
	IMMEDIATE,
	EXCLUSIVE,
};

class Select;
class SQLConn;

class AutoTransaction {
public:
	AutoTransaction() = delete;
	AutoTransaction(const SQLConn &conn,
			TransactionType type = TransactionType::DEFERRED);
	~AutoTransaction() { end(); }

	AutoTransaction(const AutoTransaction &) = delete;
	AutoTransaction &operator=(const AutoTransaction &) = delete;

	AutoTransaction(AutoTransaction &&other) noexcept : m_conn(other.m_conn) {
		other.m_conn = nullptr;
	}

	AutoTransaction &operator=(AutoTransaction &&other) noexcept {
		if (this != &other) {
			end();
			m_conn = other.m_conn;
			other.m_conn = nullptr;
		}

		return *this;
	}

	bool operator!() const { return !m_conn; }
	operator bool() const { return m_conn; }
private:
	void end();
	const SQLConn *m_conn;
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

	bool attach(const std::filesystem::path &dbFile,
		    std::string_view dbName) const noexcept;

	bool begin(TransactionType type = TransactionType::DEFERRED) const noexcept;
	bool end() const noexcept;
	AutoTransaction beginAuto(TransactionType type = TransactionType::DEFERRED) const noexcept {
		return AutoTransaction(*this, type);
	}

	std::string lastError() const { return m_lastError.lastError(); }
	int lastErrorCode() const { return m_lastErrorCode; }
	int lastErrorCodeExt() const { return m_lastErrorCodeExt; }

protected:
	SQLConn() : m_flags(0), m_lastErrorCode(0), m_lastErrorCodeExt(0) {}

	using BindVal = std::variant<std::monostate, int, unsigned, std::string, std::string_view>;
	using Binding = std::vector<std::pair<std::string, BindVal>>;
	using ColumnTypes = std::vector<std::type_index>;
	using Column = std::variant<int, std::string>;
	using Row = std::vector<Column>;
	using SelectResult = std::vector<Row>;

	enum TableFlags : unsigned {
		TABLE_TEMPORARY = 1u << 0,
	};

	struct TableEntry {
		std::string name;
		std::vector<std::string> columns;
		unsigned flags = 0u;
	};

	struct SelectEntry {
		std::reference_wrapper<Select> ref;
		std::string stmt;
		ColumnTypes columnTypes;
	};

	using Tables = std::vector<TableEntry>;
	using Indices = std::vector<std::pair<std::string, std::string>>;
	using Triggers = Indices;
	using Views = Indices;
	using Statements = std::vector<std::pair<std::reference_wrapper<SQLStmtHolder>, std::string>>;
	using Selects = std::vector<SelectEntry>;

	bool createTables(const Tables &tables) const noexcept;
	bool createIndices(const Indices &indices) const noexcept;
	bool createTriggers(const Triggers &triggers) const noexcept;
	bool createViews(const Views &views) const noexcept;

	bool prepareStatement(std::string_view sql, SQLStmtHolder &stmt) const noexcept;
	bool prepareStatements(const Statements &stmts) const noexcept;
	bool prepareSelects(const Selects &sels) const noexcept;

	bool bind(const SQLStmtHolder &ins, const std::string &key,
		  const BindVal &val, bool transient = false) const noexcept;
	bool bind(const SQLStmtHolder &ins, const Binding &binding,
		  bool transient = false) const noexcept;
	bool step(const SQLStmtHolder &ins, uint64_t *affected = nullptr,
		  bool *uniqueError = nullptr) const noexcept;
	bool insert(const SQLStmtHolder &ins, const Binding &binding = {},
		    uint64_t *affected = nullptr) const noexcept;

	std::optional<SQLConn::SelectResult>
	select(const SQLStmtHolder &sel, const Binding &binding,
	       const ColumnTypes &columns) const noexcept;

	static BindVal valueOrNull(bool cond, BindVal val) {
		return cond ? std::move(val) : std::monostate();
	}

	SlHelpers::LastError &setError(int ret, std::string_view error,
				       bool errmsg = false) const;

	SQLHolder sqlHolder;
	unsigned int m_flags;
	mutable SlHelpers::LastError m_lastError;
	mutable int m_lastErrorCode;
	mutable int m_lastErrorCodeExt;
private:
	static int busyHandler(void *, int count);
	void dumpBinding(const Binding &binding) const noexcept;
};

class Select {
public:
	Select() = delete;
	Select(const SQLConn &sqlConn) : m_sqlConn(sqlConn) {}

	bool prepare(std::string_view sql, const SQLConn::ColumnTypes &columns) noexcept {
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

inline AutoTransaction::AutoTransaction(const SQLConn &conn, TransactionType type)
	: m_conn(conn.begin(type) ? &conn : nullptr)
{
}

inline void AutoTransaction::end()
{
	if (m_conn) {
		m_conn->end();
		m_conn = nullptr;
	}
}

}
