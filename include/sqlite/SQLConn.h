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

/**
 * @brief Flags to be used for SQLConn::open()
 */
enum OpenFlags : unsigned {
	CREATE				= 1 << 0,
	NO_FOREIGN_KEY			= 1 << 1,
	ERROR_ON_UNIQUE_CONSTRAINT	= 1 << 2,
};

/**
 * @brief Transaction types used for SQLConn::begin()
 */
enum struct TransactionType {
	DEFERRED,
	IMMEDIATE,
	EXCLUSIVE,
};

class Select;
class SQLConn;

/**
 * @brief Begin a transaction in the constructor and end in the destructor
 */
class AutoTransaction {
public:
	AutoTransaction() = delete;
	/// @brief Begin an auto transaction
	AutoTransaction(const SQLConn &conn,
			TransactionType type = TransactionType::DEFERRED);
	~AutoTransaction() { end(); }

	AutoTransaction(const AutoTransaction &) = delete;
	AutoTransaction &operator=(const AutoTransaction &) = delete;

	/// @brief Move constructor
	AutoTransaction(AutoTransaction &&other) noexcept : m_conn(other.m_conn) {
		other.m_conn = nullptr;
	}
	/// @brief Move assignment
	AutoTransaction &operator=(AutoTransaction &&other) noexcept {
		if (this != &other) {
			end();
			m_conn = other.m_conn;
			other.m_conn = nullptr;
		}

		return *this;
	}

	/// @brief Test whether AutoTransaction is valid
	bool operator!() const { return !m_conn; }
	/// @brief Test whether AutoTransaction is valid
	operator bool() const { return m_conn; }
private:
	void end();
	const SQLConn *m_conn;
};

/**
 * @brief SQLite3 connection (the core class)
 */
class SQLConn {
	friend class Select;
public:
	/**
	 * @brief Open a database connection (openDB() + createDB() + prepDB())
	 * @param dbFile Path to the database
	 * @param flags Flags to use (like OpenFlags::CREATE)
	 * @return true on success.
	 */
	bool open(const std::filesystem::path &dbFile, unsigned int flags = 0)
	{
		if (!openDB(dbFile, flags) ||
				!createDB() ||
				!prepDB())
			return false;

		return true;
	}

	/**
	 * @brief Just open a database file
	 * @param dbFile Path to the database
	 * @param flags Flags to use (like OpenFlags::CREATE)
	 * @return true on success.
	 */
	bool openDB(const std::filesystem::path &dbFile, unsigned int flags = 0) noexcept;

	/**
	 * @brief Creates tables, views, triggers and such.
	 * @return true on success.
	 *
	 * Override this if you want to create some structures in the database.
	 */
	virtual bool createDB() { return true; }

	/**
	 * @brief Prepares statements
	 * @return true on success.
	 *
	 * Override this if you want to use some statements.
	 */
	virtual bool prepDB() { return true; }

	/**
	 * @brief Attach another database using ATTACH
	 * @param dbFile Path to another DB
	 * @param dbName Name to use for this attached database
	 * @return true on success.
	 */
	bool attach(const std::filesystem::path &dbFile,
		    std::string_view dbName) const noexcept;

	/**
	 * @brief Begin a transaction
	 * @param type Kind of transaction
	 * @return true on success.
	 */
	bool begin(TransactionType type = TransactionType::DEFERRED) const noexcept;

	/**
	 * @brief End a transaction
	 * @return true on success.
	 */
	bool end() const noexcept;

	/**
	 * @brief Begin a transaction which is automatically ended when the returned object dies
	 * @param type Kind of transaction
	 * @return AutoTransaction which ends the transation when destructed.
	 */
	AutoTransaction beginAuto(TransactionType type = TransactionType::DEFERRED) const noexcept {
		return AutoTransaction(*this, type);
	}

	/// @brief Return the last error string if some
	std::string lastError() const { return m_lastError.lastError(); }
	/// @brief Return the last error number
	int lastErrorCode() const { return m_lastError.get<0>(); }
	/// @brief Return the last extended error number
	int lastErrorCodeExt() const { return m_lastError.get<1>(); }

protected:
	SQLConn() : m_flags(0) {}

	/// @brief Bind value (SQL's null, number, string)
	using BindVal = std::variant<std::monostate, int, unsigned, std::string, std::string_view>;
	/// @brief Bind name -> bind value
	using Binding = std::vector<std::pair<std::string, BindVal>>;
	/// @brief Types of SELECT result
	using ColumnTypes = std::vector<std::type_index>;
	/// @brief One column returned by SELECT
	using Column = std::variant<int, std::string>;
	/// @brief One row returned by SELECT (ie. list of Columns)
	using Row = std::vector<Column>;
	/// @brief Complete SELECT result
	using SelectResult = std::vector<Row>;

	/// @brief Flags for TableEntry::flags
	enum TableFlags : unsigned {
		TABLE_TEMPORARY = 1u << 0,
	};

	/// @brief A table to be created by createTables()
	struct TableEntry {
		/// @brief Name of the table
		std::string name;
		/// @brief List of columns
		std::vector<std::string> columns;
		/// @brief See TableFlags
		unsigned flags = 0u;
	};

	/// @brief One Select to be preapred by prepareSelects()
	struct SelectEntry {
		/// @brief Reference to the Select to be prepared
		std::reference_wrapper<Select> ref;
		/// @brief SQL string to be prepared
		std::string stmt;
		/// @brief Types of the SELECT result
		ColumnTypes columnTypes;
	};

	/// @brief Tables to be created by createTables()
	using Tables = std::vector<TableEntry>;
	/// @brief Indices to be created by createIndices()
	using Indices = std::vector<std::pair<std::string, std::string>>;
	/// @brief Triggers to be created by createTriggers()
	using Triggers = Indices;
	/// @brief Views to be created by createViews()
	using Views = Indices;
	/// @brief Statements to be prepared by prepareStatements()
	using Statements = std::vector<std::pair<std::reference_wrapper<SQLStmtHolder>, std::string>>;
	/// @brief Selects to be preapred by prepareSelects()
	using Selects = std::vector<SelectEntry>;

	/// @brief Create tables in the DB as specified in \p tables
	bool createTables(const Tables &tables) const noexcept;
	/// @brief Create indices in the DB as specified in \p indices
	bool createIndices(const Indices &indices) const noexcept;
	/// @brief Create triggers in the DB as specified in \p triggers
	bool createTriggers(const Triggers &triggers) const noexcept;
	/// @brief Create views in the DB as specified in \p views
	bool createViews(const Views &views) const noexcept;

	/// @brief Prepate one \p sql string into \p stmt
	bool prepareStatement(std::string_view sql, SQLStmtHolder &stmt) const noexcept;
	/// @brief Prepate all statements as specified in \p stmts
	bool prepareStatements(const Statements &stmts) const noexcept;
	/// @brief Prepate all selects as specified in \p sels
	bool prepareSelects(const Selects &sels) const noexcept;

	/// @brief Bind one value \p val into \p key of the statement \p ins
	bool bind(const SQLStmtHolder &ins, const std::string &key,
		  const BindVal &val, bool transient = false) const noexcept;
	/// @brief Bind one \p binding for the statement \p ins
	bool bind(const SQLStmtHolder &ins, const Binding &binding,
		  bool transient = false) const noexcept;
	/// @brief Do one step of the statement \p ins
	bool step(const SQLStmtHolder &ins, uint64_t *affected = nullptr,
		  bool *uniqueError = nullptr) const noexcept;
	/// @brief Bind, step, and reset the statement \p ins, using the passed \p binding
	bool insert(const SQLStmtHolder &ins, const Binding &binding = {},
		    uint64_t *affected = nullptr) const noexcept;

	/// @brief Perform one SELECT (\p sel), using the passed \p binding and types of \p columns
	std::optional<SQLConn::SelectResult>
	select(const SQLStmtHolder &sel, const Binding &binding,
	       const ColumnTypes &columns) const noexcept;

	/// @brief A helper to build a null BindVal if \p cond does not hold, \p val otherwise
	static BindVal valueOrNull(bool cond, BindVal val) {
		return cond ? std::move(val) : std::monostate();
	}

	/// @brief Store a string + 2 ints
	using LastError = SlHelpers::LastErrorStream<int, int>;

	/// @brief Set last error to \p ret and \p error
	LastError &setError(int ret, std::string_view error, bool errmsg = false) const;

	/// @brief The DB connection
	SQLHolder sqlHolder;
	/// @brief OpenFlags
	unsigned int m_flags;
	/// @brief The last error + error code + extended error code
	mutable LastError m_lastError;
private:
	static int busyHandler(void *, int count);
	void dumpBinding(const Binding &binding) const noexcept;
};

/**
 * @brief Special class for SELECT statements
 */
class Select {
public:
	Select() = delete;

	/**
	 * @brief Constructs an empty Select
	 * @param sqlConn Connection this SELECT should be bound to
	 */
	Select(const SQLConn &sqlConn) : m_sqlConn(sqlConn) {}

	/**
	 * @brief Prepare this SELECT with \p sql
	 * @param sql SQL SELECT string
	 * @param columns Types of returned columns by this SELECT
	 * @return true on success.
	 */
	bool prepare(std::string_view sql, const SQLConn::ColumnTypes &columns) noexcept {
		m_resultTypes = columns;
		return m_sqlConn.prepareStatement(sql, m_select);
	}

	/**
	 * @brief Perform the actual SELECT
	 * @param binding Values to pass to prepared SELECT
	 * @return Rows with data, typed by \p columns in prepare()
	 */
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
