// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLSQLITE_SQLITESMART_H
#define SLSQLITE_SQLITESMART_H

#include "../helpers/Unique.h"

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

namespace SlSqlite {

using SQLHolder = SlHelpers::UniqueHolder<sqlite3>;
using SQLStmtHolder = SlHelpers::UniqueHolder<sqlite3_stmt>;

struct SQLStmtResetter {
	SQLStmtResetter(sqlite3 *sql, sqlite3_stmt *stmt) : sql(sql), stmt(stmt) { }
	~SQLStmtResetter();
private:
	sqlite3 *sql;
	sqlite3_stmt *stmt;
};

}

#endif
