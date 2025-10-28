// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLKERNCVS_BRANCHES_H
#define SLKERNCVS_BRANCHES_H

#include <optional>
#include <string>
#include <vector>

namespace SlKernCVS {

class Branches {
public:
	using BranchesList = std::vector<std::string>;

	Branches() = delete;

	static BranchesList getBuildBranches(const std::string &branchesConf);
	static std::optional<BranchesList> getBuildBranches();
private:
	static bool isExcluded(const std::string &branch);
};

}

#endif
