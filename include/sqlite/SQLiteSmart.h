// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "../helpers/Unique.h"

struct sqlite3;
struct sqlite3_stmt;

namespace SlSqlite {

using SQLHolder = SlHelpers::UniqueHolder<sqlite3>;
using SQLStmtHolder = SlHelpers::UniqueHolder<sqlite3_stmt>;

struct SQLStmtResetter {
	SQLStmtResetter(sqlite3_stmt *stmt) : m_stmt(stmt) { }
	~SQLStmtResetter();

	int reset();
private:
	sqlite3_stmt *m_stmt;
};

}
