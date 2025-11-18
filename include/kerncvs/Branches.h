// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLKERNCVS_BRANCHES_H
#define SLKERNCVS_BRANCHES_H

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SlKernCVS {

struct BranchProps {
	using BranchesList = std::vector<std::string>;

	bool isBuild;
	bool isPublish;
	bool isExcluded;
	BranchesList merges;
};

class Branches {
public:
	enum Filter : unsigned {
		BUILD		= 1U << 0,
		PUBLISH		= 1U << 1,
		EXCLUDED	= 1U << 2,
		ANY 		= ~0U,
	};

	using BranchesSet = std::unordered_set<std::string>;
	using BranchesList = BranchProps::BranchesList;
	using BranchesMap = std::unordered_map<std::string, BranchProps>;

	Branches() = delete;
	static Branches create(const std::string &branchesConf);
	static std::optional<Branches> create();

	const BranchesMap &map() const noexcept { return m_map; }
	auto begin() const noexcept { return m_map.begin(); }
	auto end() const noexcept { return m_map.end(); }

	BranchesList filter(unsigned included = ANY, unsigned exclude = EXCLUDED) const;
	const BranchesList &merges(const std::string &branch) const {
		return m_map.at(branch).merges;
	}
	BranchesSet mergesClosure(const std::string &branch) const;

	static BranchesList getBuildBranches(const std::string &branchesConf);
	static std::optional<BranchesList> getBuildBranches();
private:
	Branches(BranchesMap &map) : m_map(std::move(map)) {}

	void dfs(const std::string &u, BranchesSet &visited) const;

	BranchesMap m_map;

	static bool isExcluded(const std::string_view &branch);
};

}

#endif
