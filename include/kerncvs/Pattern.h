// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "../git/PathSpec.h"

namespace SlKernCVS {

struct Pattern {
	Pattern() = delete;

	unsigned match(const std::filesystem::path &path) const {
		if (m_pathspec.matchesPath(path))
			return m_weight;
		return 0;
	}

	static std::optional<Pattern> create(std::string pattern);
private:
	SlGit::PathSpec m_pathspec;
	unsigned m_weight;

	Pattern(SlGit::PathSpec pathspec, unsigned weight) : m_pathspec(std::move(pathspec)),
		m_weight(weight) {}

	static constexpr unsigned pattern_weight(std::string_view pattern);
};

}
