// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>

namespace SlGit {
class Commit;
class Repo;
}

namespace SlKernCVS {

class PatchesAuthors {
public:
	using Map = std::map<std::string, std::map<std::string, unsigned int>>;
	using InsertUser = std::function<bool (const std::string &)>;
	using InsertUFMap = std::function<bool (const std::string &, const std::filesystem::path &,
		unsigned, unsigned)>;

	PatchesAuthors(const SlGit::Repo &repo, bool dumpRefs, bool reportUnhandled) :
		repo(&repo), dumpRefs(dumpRefs), reportUnhandled(reportUnhandled)
	{}

	bool processAuthors(const SlGit::Commit &commit, const InsertUser &insertUser,
			    const InsertUFMap &insertUFMap);
private:
	PatchesAuthors() : repo(nullptr), dumpRefs(false), reportUnhandled(false) {}
	friend void testProcessPatch();

	int processPatch(const std::filesystem::path &file, const std::string &content);

	const SlGit::Repo *repo;
	const bool dumpRefs;
	const bool reportUnhandled;
	Map m_HoH;
	Map m_HoHReal;
	Map m_HoHRefs;
};

}
