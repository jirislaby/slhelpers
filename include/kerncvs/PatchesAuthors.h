// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLKERNCVS_PATCHESAUTHORS_H
#define SLKERNCVS_PATCHESAUTHORS_H

#include <filesystem>
#include <functional>
#include <map>
#include <regex>
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
	static const std::regex REInteresting;
	static const std::regex REFalse;
	static const std::regex REGitFixes;
	static const std::regex REInvalRef;
	Map m_HoH;
	Map m_HoHReal;
	Map m_HoHRefs;
};

}

#endif
