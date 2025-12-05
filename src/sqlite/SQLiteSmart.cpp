// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <sqlite3.h>

#include "sqlite/SQLiteSmart.h"

using namespace SlSqlite;

template<>
void SlHelpers::Deleter<sqlite3>::operator()(sqlite3 *sql) const
{
	sqlite3_close(sql);
}

template<>
void SlHelpers::Deleter<sqlite3_stmt>::operator()(sqlite3_stmt *stmt) const
{
	sqlite3_finalize(stmt);
}

SQLStmtResetter::~SQLStmtResetter()
{
	auto ret = reset();
	if (ret != SQLITE_OK && ret != SQLITE_CONSTRAINT) {
		std::cerr << "stmt reset failed (" << __LINE__ << "): " <<
			     sqlite3_errstr(ret) << " -> " <<
			     sqlite3_errmsg(sql) << "\n";
	}
}

int SQLStmtResetter::reset()
{
	auto ret = sqlite3_reset(stmt);
	stmt = nullptr;
	return ret;
}
