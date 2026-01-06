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
	/// @brief List of branches
	using BranchesList = std::vector<std::string>;

	/// @brief Marked as build in branches.conf
	bool isBuild;
	/// @brief Marked as publish in branches.conf
	bool isPublish;
	/// @brief One of master, stable, vanilla, or similar branches
	bool isExcluded;

	/// @brief What immediate branches this branch merges
	BranchesList merges;
};

/**
 * @brief Parse branches.conf into a map of branch -> properties (BranchProps)
 */
class Branches {
public:
	/// @brief Constants used for filter()
	enum Filter : unsigned {
		BUILD		= 1U << 0,
		PUBLISH		= 1U << 1,
		EXCLUDED	= 1U << 2,
		ANY 		= ~0U,
	};

	/// @brief A set of branches
	using BranchesSet = std::unordered_set<std::string>;
	/// @brief A list of branches
	using BranchesList = BranchProps::BranchesList;
	/// @brief Branch -> BranchProps mapping
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

	/**
	 * @brief Obtain whole branch map
	 * @return Branch mapping.
	 */
	const BranchesMap &map() const noexcept { return m_map; }

	/**
	 * @brief Obtain begin iterator of branches
	 * @return Begin iterator.
	 */
	auto begin() const noexcept { return m_map.begin(); }
	/**
	 * @brief Obtain end iterator of branches
	 * @return End iterator.
	 */
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
