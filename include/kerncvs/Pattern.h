// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "../git/PathSpec.h"

namespace SlKernCVS {

/**
 * @brief Holds git patterns and reports weights if matched.
 */
struct Pattern {
	Pattern() = delete;

	/**
	 * @brief Match a \p path against the stored patterns
	 * @param path A path to match
	 * @return weight of the matches pattern or 0 if not found.
	 */
	unsigned match(const std::filesystem::path &path) const {
		if (m_pathspec.matchesPath(path))
			return m_weight;
		return 0;
	}

	/**
	 * @brief Build a (git) Pattern
	 * @param pattern A pattern to build from
	 * @return A Pattern if successful, otherwise nullopt.
	 */
	static std::optional<Pattern> create(std::string pattern);
private:
	SlGit::PathSpec m_pathspec;
	unsigned m_weight;

	Pattern(SlGit::PathSpec pathspec, unsigned weight) : m_pathspec(std::move(pathspec)),
		m_weight(weight) {}

	static constexpr unsigned pattern_weight(std::string_view pattern);
};

}
