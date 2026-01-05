// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "../helpers/Unique.h"

struct sqlite3;
struct sqlite3_stmt;

namespace SlSqlite {

using SQLHolder = SlHelpers::UniqueHolder<sqlite3>;
using SQLStmtHolder = SlHelpers::UniqueHolder<sqlite3_stmt>;

/**
 * @brief Resets SQLite statement after use for re-use
 */
struct SQLStmtResetter {
	/**
	 * @brief Set up \p stmt to be reset in the destructor
	 * @param stmt The statement to reset
	 */
	SQLStmtResetter(sqlite3_stmt *stmt) : m_stmt(stmt) { }
	~SQLStmtResetter();

	/**
	 * @brief Reset the statement
	 * @return 0 on success.
	 *
	 * This is usually done only by the destructor.
	 */
	int reset();
private:
	sqlite3_stmt *m_stmt;
};

}
