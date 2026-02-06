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

/**
 * @brief PatchesAuthors parses all patches in all ("build") branches in the kernel-source tree and
 * reports who touched what file in the linux (upstream) tree.
 *
 * The reporting happens via the provided callbacks.
 */
class PatchesAuthors {
public:
	/// @brief E-mail -> file -> count mapping
	using Map = std::map<std::string, std::map<std::string, unsigned int>>;
	/// @brief A callback invoked for e-mail
	using InsertUser = std::function<bool (const std::string &email)>;
	/// @brief A callback invoked for e-mail, file, and counts of git-fixes and real changes
	using InsertUFMap = std::function<bool (const std::string &email,
		const std::filesystem::path &file, unsigned gitFixes, unsigned realFixes)>;

	/**
	 * @brief PatchesAuthors constructor
	 * @param repo KernCVS repository to search in
	 * @param dumpRefs Should References: be dumped to stdout?
	 * @param reportUnhandled Should unhandled @@suse e-mail be reported to stderr?
	 */
	PatchesAuthors(const SlGit::Repo &repo, bool dumpRefs, bool reportUnhandled) :
		repo(&repo), dumpRefs(dumpRefs), reportUnhandled(reportUnhandled)
	{}

	/**
	 * @brief The real work function of this class
	 * @param commit The commit to walk
	 * @param insertUser Callback to invoke for a user
	 * @param insertUFMap Callback to invoke for a user, path, and (all+non-git-fixes) counts
	 * @return true on success.
	 */
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
