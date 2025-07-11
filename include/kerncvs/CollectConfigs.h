// SPDX-License-Identifier: GPL-2.0-only

#ifndef COLLECTCONFIGS_H
#define COLLECTCONFIGS_H

#include <filesystem>
#include <functional>
#include <string>

/*namespace SlGit {
class Commit;
class Repo;
}*/
#include "git/Git.h"

namespace SlKernCVS {

class CollectConfigs {
public:
	using InsertUser = std::function<int (const std::string &)>;
	using InsertUFMap = std::function<int (const std::string &, const std::filesystem::path &,
		unsigned, unsigned)>;

	CollectConfigs(const SlGit::Repo &repo) : repo(repo) {}

	int collectConfigs(const SlGit::Commit &commit);

private:
	int processFlavor(const std::string &arch, const std::string &flavor,
			  const SlGit::TreeEntry &treeEntry);
	const SlGit::Repo &repo;
	//static const std::regex REInvalRef;
};

}

#endif

