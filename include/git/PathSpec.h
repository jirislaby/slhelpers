// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_PATHSPEC_H
#define SLGIT_PATHSPEC_H

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

class PathSpecMatchList {
	using GitTy = git_pathspec_match_list;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
	friend class PathSpec;
public:
	PathSpecMatchList() = delete;

	size_t entrycount() const noexcept {
		return git_pathspec_match_list_entrycount(matchList());
	}
	std::string entry(size_t pos) const noexcept {
		return git_pathspec_match_list_entry(matchList(), pos);
	}

	const git_diff_delta *diffEntry(size_t pos) const noexcept {
		return git_pathspec_match_list_diff_entry(matchList(), pos);
	}

	size_t failedEntrycount() const noexcept {
		return git_pathspec_match_list_failed_entrycount(matchList());
	}
	std::string failedEntry(size_t pos) const noexcept {
		return git_pathspec_match_list_failed_entry(matchList(), pos);
	}

	GitTy *matchList() const noexcept { return m_matchList.get(); }
	operator GitTy *() const noexcept { return matchList(); }
private:
	explicit PathSpecMatchList(GitTy *matchList) noexcept : m_matchList(matchList) { }

	Holder m_matchList;
};

class PathSpec {
	using GitTy = git_pathspec;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	PathSpec() = delete;

	static std::optional<PathSpec> create(const std::vector<std::string> &pathSpec) noexcept;

	bool matchesPath(const std::string &path,
			 uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept {
		return git_pathspec_matches_path(m_pathSpec, flags, path.c_str());
	}

	std::optional<PathSpecMatchList> matchWorkdir(const Repo &repo,
						      uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;
	std::optional<PathSpecMatchList> matchIndex(const Index &index,
						    uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;
	std::optional<PathSpecMatchList> matchTree(const Tree &tree,
						   uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;
	std::optional<PathSpecMatchList> matchDiff(const Diff &diff,
						   uint32_t flags = GIT_PATHSPEC_DEFAULT) const noexcept;

	GitTy *pathSpec() const noexcept { return m_pathSpec.get(); }
	operator GitTy *() const noexcept { return pathSpec(); }
private:
	explicit PathSpec(GitTy *pathSpec) noexcept : m_pathSpec(pathSpec) { }

	Holder m_pathSpec;
};

}

#endif
