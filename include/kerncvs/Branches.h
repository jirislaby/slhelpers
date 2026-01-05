// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../helpers/String.h"

namespace SlKernCVS {

/**
 * @brief Properties of a branch stored in Branches
 */
struct BranchProps {
	using BranchesList = std::vector<std::string>;

	bool isBuild;
	bool isPublish;
	bool isExcluded;
	BranchesList merges;
};

/**
 * @brief Parse branches.conf into a map of branch -> properties (BranchProps)
 */
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
	using BranchesMap = std::unordered_map<std::string, BranchProps, SlHelpers::String::Hash,
		SlHelpers::String::Eq>;

	Branches() = delete;
	/**
	 * @brief Parse provided \p branchesConf into Branches
	 * @param branchesConf branches.conf content to parse
	 * @return Branches parsed from \p branchesConf
	 */
	static Branches create(std::string_view branchesConf) noexcept;
	/**
	 * @brief Download branches.conf and parse it into Branches
	 * @return Branches if successful or nullopt.
	 */
	static std::optional<Branches> create();

	const BranchesMap &map() const noexcept { return m_map; }
	auto begin() const noexcept { return m_map.begin(); }
	auto end() const noexcept { return m_map.end(); }

	/**
	 * @brief Obtain BranchesList according to a filter specified by \p include and \p exclude
	 * @param include Properties of branches to include in the output
	 * @param exclude Properties of branches to exclude from the output
	 * @return BranchesList according to the specified filter
	 */
	BranchesList filter(unsigned include = ANY, unsigned exclude = EXCLUDED) const;

	/**
	 * @brief Immediate branches that the specified \p branch merges
	 * @param branch Branch to find children of
	 * @return BranchesList of branches merged into \p branch.
	 */
	const BranchesList &merges(std::string_view branch) const {
		return m_map.find(branch)->second.merges;
	}
	/**
	 * @brief Closure of branches that the specified \p branch merges
	 * @param branch Branch to find children of
	 * @return Transitive BranchesSet of branches merged into \p branch.
	 */
	BranchesSet mergesClosure(std::string_view branch) const;

	/**
	 * @brief Convert \p branchesConf to a list of branches which are built
	 * @param branchesConf branches.conf to parse
	 * @return List of built branches.
	 */
	static BranchesList getBuildBranches(std::string_view branchesConf) noexcept;

	/**
	 * @brief Download branches.conf and convert it to a list of branches which are built
	 * @return List of built branches.
	 */
	static std::optional<BranchesList> getBuildBranches();
private:
	Branches(BranchesMap &map) : m_map(std::move(map)) {}

	void dfs(std::string_view u, BranchesSet &visited) const;

	BranchesMap m_map;

	static bool isExcluded(std::string_view branch);
};

}
