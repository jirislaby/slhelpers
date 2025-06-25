// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <sqlite3.h>

#include "sqlite/SQLiteSmart.h"

using namespace SlSqlite;

SQLHolder::SQLHolder(sqlite3 *sql) : SQLUnique(sql, [](sqlite3 *sql) {
		sqlite3_close(sql);
	})
{
}

SQLStmtHolder::SQLStmtHolder(sqlite3_stmt *stmt) : SQLStmtUnique(stmt, [](sqlite3_stmt *stmt) {
		sqlite3_finalize(stmt);
	})
{
}

SQLStmtResetter::~SQLStmtResetter()
{
	int ret = sqlite3_reset(stmt);
	if (ret != SQLITE_OK && ret != SQLITE_CONSTRAINT) {
		std::cerr << "stmt reset failed (" << __LINE__ << "): " <<
			     sqlite3_errstr(ret) << " -> " <<
			     sqlite3_errmsg(sql) << "\n";
	}
}
