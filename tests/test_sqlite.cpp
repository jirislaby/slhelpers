// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <iostream>
#include <optional>

#include "helpers/Color.h"
#include "sqlite/SQLConn.h"

#include "helpers.h"

using namespace SlSqlite;
using Clr = SlHelpers::Color;

namespace {

class SQLConn : public SlSqlite::SQLConn {
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
			{ delPerson, "DELETE FROM person;" },
		};

		return  prepareStatements(stmts) &&
			selPerson.prepare("SELECT person.name, age, address.street "
					  "FROM person "
					  "LEFT JOIN address ON person.address = address.id "
					  "WHERE person.name LIKE :name "
					  "ORDER BY person.id;",
					  { typeid(std::string), typeid(int), typeid(std::string) });
	}

	bool badInsertAddress(const std::string &street) const {
		return insert(insAddress, { { ":streetFoo", street } });
	}

	bool insertAddress(const std::string &street) const {
		return insert(insAddress, { { ":street", street } });
	}

	bool insertPerson(const std::string &name, const int age, const std::string &street,
			  uint64_t *affected = nullptr) const {
		return insert(insPerson, {
				      { ":name", name },
				      { ":age", age },
				      { ":street", street },
			      }, affected);
	}

	std::optional<SlSqlite::SQLConn::SelectResult>
	getPersons(const std::string &name) const
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

const struct {
	std::string name;
	int age;
	std::string addr;
} people[] = {
	{ "John Smith", 21, "Whale street 21" },
	{ "John Cagliari", 25, "Down street 105" },
};

void testInsert(const SQLConn &db)
{
	uint64_t affected;
	for (const auto &e: people) {
		affected = ~0ULL;
		assert(db.insertAddress(e.addr));
		assert(db.insertPerson(e.name, e.age, e.addr, &affected));
		assert(affected == 1);
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
		assert(res.size() == 2);
		assert(res[0].size() == 3);
		assert(res[1].size() == 3);

		assert(std::get<std::string>(res[1][0]) == people[1].name);
	}

	{
		const auto resOpt = db.getPersons("non-existant");
		assert(resOpt);
		assert(resOpt->size() == 0);
	}
}

void testDelete(const SQLConn &db)
{
	uint64_t affected;
	assert(db.delPersons(&affected));
	assert(affected == 2);
}

}

int main()
{
	const auto tmpDir = THelpers::getTmpDir();
	const auto db = testOpen(tmpDir);
	testInsert(db);
	testSelect(db);
	testDelete(db);
	std::filesystem::remove_all(tmpDir);

	return 0;
}

