// SPDX-License-Identifier: GPL-2.0-only

#ifndef SQLITESMART_H
#define SQLITESMART_H

#include <memory>

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

namespace SQL {

using SQLUnique = std::unique_ptr<sqlite3, void (*)(sqlite3 *)>;
using SQLStmtUnique = std::unique_ptr<sqlite3_stmt, void (*)(sqlite3_stmt *)>;

struct SQLHolder : public SQLUnique {
	SQLHolder() : SQLHolder(nullptr) { }

	SQLHolder(sqlite3 *sql);

	operator sqlite3 *() { return get(); }
};

struct SQLStmtHolder : public SQLStmtUnique {
	SQLStmtHolder() : SQLStmtHolder(nullptr) { }

	SQLStmtHolder(sqlite3_stmt *stmt);

	operator sqlite3_stmt *() { return get(); }
};

struct SQLStmtResetter {
	SQLStmtResetter(sqlite3 *sql, sqlite3_stmt *stmt) : sql(sql), stmt(stmt) { }
	~SQLStmtResetter();
private:
	sqlite3 *sql;
	sqlite3_stmt *stmt;
};

}

#endif
