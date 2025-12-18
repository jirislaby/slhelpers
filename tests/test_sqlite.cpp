// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <cstring>
#include <iostream>
#include <optional>
#include <sqlite3.h>

#include "helpers/Color.h"
#include "sqlite/SQLConn.h"
#include "../src/sqlite/PtrStore.h"

#include "helpers.h"

using namespace SlSqlite;
using Clr = SlHelpers::Color;

namespace {

class SQLConnTemp : public SlSqlite::SQLConn {
public:
	SQLConnTemp() : selPersonTemp(*this) {}

	virtual bool prepDB() override {
		return selPersonTemp.prepare("SELECT 1 FROM personTemp;", { typeid(int) });
	}

	std::optional<SlSqlite::SQLConn::SelectResult> getPersonsTemp() const {
		return selPersonTemp.select({});
	}

private:
	SlSqlite::Select selPersonTemp;
};

class SQLConn : public SQLConnTemp {
public:
	SQLConn() : selPerson(*this) {}
	virtual bool createDB() override {
		static const Tables create_tables {
			{ "address", {
				"id INTEGER PRIMARY KEY",
				"street TEXT NOT NULL UNIQUE",
			}},
			{ "person", {
				"id INTEGER PRIMARY KEY",
				"name TEXT NOT NULL UNIQUE",
				"age INTEGER NOT NULL",
				"address INTEGER NOT NULL REFERENCES address(id)",
			}},
			{ "personTemp", {
				"id INTEGER PRIMARY KEY",
				"name TEXT NOT NULL",
				"age INTEGER NOT NULL",
				"street TEXT NOT NULL",
				"UNIQUE(name, street)"
			}, TABLE_TEMPORARY },
		};

		return createTables(create_tables);
	}

	virtual bool prepDB() override {
		static const Statements stmts {
			{ insAddress, "INSERT INTO address(street) VALUES (:street);" },
			{ insPerson, "INSERT INTO person(name, age, address) "
				     "SELECT :name, :age, address.id "
				     "FROM address "
				     "WHERE address.street = :street;" },
			{ insPersonTemp,	"INSERT INTO personTemp(name, age, street) "
						"SELECT :name, :age, :street;" },
			{ moveAddress,	"INSERT INTO address(street) "
					"SELECT DISTINCT street FROM personTemp;" },
			{ movePerson,	"INSERT INTO person(name, age, address) "
					"SELECT personTemp.name, personTemp.age, address.id "
					"FROM personTemp "
					"JOIN address ON personTemp.street = address.street;" },
			{ delPerson, "DELETE FROM person;" },
		};
		static const Selects sels {
			{ selPerson, "SELECT person.name, age, address.street "
				     "FROM person "
				     "LEFT JOIN address ON person.address = address.id "
				     "WHERE person.name LIKE :name "
				     "ORDER BY person.id;",
				{ typeid(std::string), typeid(int), typeid(std::string) }},
		};

		return	SQLConnTemp::prepDB() && prepareStatements(stmts) && prepareSelects(sels);
	}

	bool badInsertAddress(std::string_view street) const {
		return insert(insAddress, { { ":streetFoo", street } });
	}

	bool insertAddress(std::string_view street) const {
		return insert(insAddress, { { ":street", street } });
	}

	bool insertPerson(std::string_view name, const int age, std::string_view street,
			  uint64_t *affected = nullptr) const {
		return insert(insPerson, {
				      { ":name", name },
				      { ":age", age },
				      { ":street", street },
			      }, affected);
	}

	bool insertPersonTemp(std::string_view name, const int age, std::string_view street,
			      uint64_t *affected = nullptr) const {
		return insert(insPersonTemp, {
				      { ":name", name },
				      { ":age", age },
				      { ":street", street },
			      }, affected);
	}

	bool movePersonTemp(uint64_t *affected = nullptr) const {
		uint64_t aff1, aff2;
		auto ret = insert(moveAddress, {}, &aff1) && insert(movePerson, {}, &aff2);
		if (ret && affected)
			*affected = aff1 + aff2;
		return ret;
	}

	std::optional<SlSqlite::SQLConn::SelectResult>
	getPersons(std::string_view name) const
	{
		return selPerson.select({ { ":name", name } });
	}

	bool delPersons(uint64_t *affected = nullptr) const {
		return insert(delPerson, {}, affected);
	}


	explicit operator bool() const { return sqlHolder.operator bool(); }
private:
	SlSqlite::SQLStmtHolder insAddress;
	SlSqlite::SQLStmtHolder insPerson;
	SlSqlite::SQLStmtHolder insPersonTemp;
	SlSqlite::SQLStmtHolder moveAddress;
	SlSqlite::SQLStmtHolder movePerson;
	SlSqlite::Select selPerson;
	SlSqlite::SQLStmtHolder delPerson;
};


SQLConn testOpen(const std::filesystem::path &tmpDir)
{
	SQLConn db;

	assert(!db.open(tmpDir / "sql.db"));
	Clr(std::cerr, Clr::GREEN) << "EXPECTED error: " << db.lastError();
	assert(db.lastError().find("unable to open database file") != std::string::npos);

	auto ret = db.open(tmpDir / "sql.db",
			   OpenFlags::CREATE | OpenFlags::ERROR_ON_UNIQUE_CONSTRAINT);
	if (!ret) {
		Clr(std::cerr, Clr::RED) << db.lastError();
		assert(false);
	}

	return db;
}

constinit const struct {
	std::string_view name;
	int age;
	std::string_view addr;
} people[] = {
	{ "John Smith", 21, "Whale street 21" },
	{ "John Cagliari", 25, "Down street 105" },
}, peopleTemp[] = {
	{ "Jane Smoth", 18, "Whole street 30" },
	{ "Jane Caliari", 35, "Dawn street 55" },
};

unsigned persons;

void testInsert(const SQLConn &db)
{
	uint64_t affected;
	{
		auto trans = db.beginAuto();
		assert(trans);
		assert(!!trans);
		for (const auto &e: people) {
			affected = ~0ULL;
			assert(db.insertAddress(e.addr));
			assert(db.insertPerson(e.name, e.age, e.addr, &affected));
			assert(affected == 1);
			persons += affected;
		}
	}

	assert(!db.insertAddress(people[0].addr));
	Clr(std::cerr, Clr::GREEN) << "EXPECTED error: " << db.lastError();
	assert(db.lastError().find("constraint failed") != std::string::npos);

	assert(!db.badInsertAddress("Some addr"));
	Clr(std::cerr, Clr::GREEN) << "EXPECTED error: " << db.lastError();
	assert(db.lastError().find("no index found") != std::string::npos);

	affected = ~0ULL;
	assert(!db.insertPerson(people[0].name, people[0].age, people[0].addr, &affected));
	assert(affected == ~0ULL);

	affected = ~0ULL;
	assert(db.insertPerson(people[1].name, people[1].age, "non-existant", &affected));
	assert(affected == 0);
}

void testTemp(const SQLConn &db)
{
	uint64_t affected;
	unsigned rows = 0;
	for (const auto &e: peopleTemp) {
		affected = ~0ULL;
		assert(db.insertPersonTemp(e.name, e.age, e.addr, &affected));
		assert(affected == 1);
		persons += affected;
		rows += affected;
	}

	affected = ~0ULL;
	assert(db.movePersonTemp(&affected));
	assert(affected == rows * 2); // one addr, one person
}

void testSelect(const SQLConn &db)
{
	{
		auto resOpt = db.getPersons(people[0].name);
		assert(resOpt);
		const auto res = std::move(*resOpt);
		assert(res.size() == 1);
		assert(res[0].size() == 3);

		assert(std::holds_alternative<std::string>(res[0][0]));
		assert(std::get<std::string>(res[0][0]) == people[0].name);

		assert(std::holds_alternative<int>(res[0][1]));
		assert(std::get<int>(res[0][1]) == people[0].age);

		assert(std::holds_alternative<std::string>(res[0][2]));
		assert(std::get<std::string>(res[0][2]) == people[0].addr);
	}

	{
		auto resOpt = db.getPersons("%");
		assert(resOpt);
		const auto res = std::move(*resOpt);
		assert(res.size() == persons);
		assert(res[0].size() == 3);
		assert(res[1].size() == 3);

		assert(std::get<std::string>(res[1][0]) == people[1].name);
		assert(std::get<std::string>(res[3][0]) == peopleTemp[1].name);
	}

	{
		const auto resOpt = db.getPersons("non-existant");
		assert(resOpt);
		assert(resOpt->size() == 0);
	}

	{
		auto resOpt = db.getPersonsTemp();
		assert(resOpt);
		const auto res = std::move(*resOpt);
		assert(std::get<int>(res[0][0]) == 1);
	}
}

void testAttach(const SQLConn &db)
{
	assert(db.attach("", "my_temp"));
}

void testDelete(const SQLConn &db)
{
	uint64_t affected;
	assert(db.delPersons(&affected));
	assert(affected == persons);
}

void testErrStore()
{
	static_assert(!std::is_copy_constructible<CharPtrStore>());
	static_assert(!std::is_copy_assignable<CharPtrStore>());
	static_assert(std::is_move_constructible<CharPtrStore>());
	static_assert(std::is_move_assignable<CharPtrStore>());
	CharPtrStore err;
	char *mem = static_cast<char *>(sqlite3_malloc(100));
	strncpy(mem, "test", 100);
	*err.ptr() = mem;
	CharPtrStore err2;
	err2 = std::move(err);
	assert(err2);
	assert(err2.str() == "test");
}

}

int main()
{
	const auto tmpDir = THelpers::getTmpDir();
	{
		const auto db = testOpen(tmpDir);
		testInsert(db);
		testTemp(db);
		testSelect(db);
		testAttach(db);
		testDelete(db);
	}
	{
		SQLConnTemp db;
		assert(!db.open(tmpDir / "sql.db"));
		assert(db.lastErrorCode() == SQLITE_ERROR);
		assert(db.lastError().find("no such table: personTemp") != std::string::npos);
	}
	testErrStore();
	std::filesystem::remove_all(tmpDir);

	return 0;
}

