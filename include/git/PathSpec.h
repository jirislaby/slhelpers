// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"

namespace SlGit {

class Diff;
class Index;
class Repo;
class Tree;

/**
 * @brief PathSpecMatchList is a representation of a git pathspecs match list
 */
class PathSpecMatchList {
	using GitTy = git_pathspec_match_list;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
	friend class PathSpec;
public:
	PathSpecMatchList() = delete;

	/// @brief Get count of entries in this PathSpecMatchList
	size_t entrycount() const noexcept {
		return git_pathspec_match_list_entrycount(matchList());
	}
	/// @brief Get an entry on \p pos-th position
	std::string entry(size_t pos) const noexcept {
		return git_pathspec_match_list_entry(matchList(), pos);
	}

	/// @brief Get a diff entry on \p pos-th position
	const git_diff_delta *diffEntry(size_t pos) const noexcept {
		return git_pathspec_match_list_diff_entry(matchList(), pos);
	}

	/// @brief Get count of failed entries in this PathSpecMatchList
	size_t failedEntrycount() const noexcept {
		return git_pathspec_match_list_failed_entrycount(matchList());
	}
	/// @brief Get a failed entry on \p pos-th position
	std::string failedEntry(size_t pos) const noexcept {
		return git_pathspec_match_list_failed_entry(matchList(), pos);
	}

	/// @brief Get the stored pointer to libgit2's git_pathspec_match_list
	GitTy *matchList() const noexcept { return m_matchList.get(); }
	/// @brief Alias for matchList() -- implicit conversion
	operator GitTy *() const noexcept { return matchList(); }
private:
	explicit PathSpecMatchList(GitTy *matchList) noexcept : m_matchList(matchList) { }

	Holder m_matchList;
};

/**
 * @brief PathSpec is a representation of git pathspecs
 */
class PathSpec {
	using GitTy = git_pathspec;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	PathSpec() = delete;

	/// @brief Create a new PathSpec, to match every path in \p pathSpec
	static std::optional<PathSpec> create(const std::vector<std::string> &pathSpec) noexcept;

	/// @brief Return true if this PathSpec matches \p path
	bool matchesPath(const std::string &path,
			 uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept {
		return git_pathspec_matches_path(m_pathSpec, flags, path.c_str());
	}

	/// @brief Return paths this PathSpec matches in the workdir
	std::optional<PathSpecMatchList> matchWorkdir(const Repo &repo,
						      uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;
	/// @brief Return paths this PathSpec matches in the \p index
	std::optional<PathSpecMatchList> matchIndex(const Index &index,
						    uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;
	/// @brief Return paths this PathSpec matches in the \p tree
	std::optional<PathSpecMatchList> matchTree(const Tree &tree,
						   uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;
	/// @brief Return paths this PathSpec matches in the \p diff
	std::optional<PathSpecMatchList> matchDiff(const Diff &diff,
						   uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;

	/// @brief Get the stored pointer to libgit2's git_pathspec
	GitTy *pathSpec() const noexcept { return m_pathSpec.get(); }
	/// @brief Alias for pathSpec() -- implicit conversion
	operator GitTy *() const noexcept { return pathSpec(); }
private:
	explicit PathSpec(GitTy *pathSpec) noexcept : m_pathSpec(pathSpec) { }

	Holder m_pathSpec;
};

}
