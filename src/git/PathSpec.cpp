// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>
#include <optional>

#include "git/Diff.h"
#include "git/Index.h"
#include "git/PathSpec.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "git/StrArray.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_pathspec>::operator()(git_pathspec *ps) const
{
	git_pathspec_free(ps);
}

template<>
void SlHelpers::Deleter<git_pathspec_match_list>::operator()(git_pathspec_match_list *ml) const
{
	git_pathspec_match_list_free(ml);
}

std::optional<PathSpec> PathSpec::create(const std::vector<std::string> &pathSpec) noexcept
{
	return Repo::MakeGit<PathSpec>(git_pathspec_new, StrArray(pathSpec));
}

std::optional<PathSpecMatchList> PathSpec::matchWorkdir(const Repo &repo,
							uint32_t flags) const noexcept
{
	return Repo::MakeGit<PathSpecMatchList>(git_pathspec_match_workdir, repo, flags,
						pathSpec());
}

std::optional<PathSpecMatchList> PathSpec::matchIndex(const Index &index, uint32_t flags) const noexcept
{
	return Repo::MakeGit<PathSpecMatchList>(git_pathspec_match_index, index, flags,
						pathSpec());
}

std::optional<PathSpecMatchList> PathSpec::matchTree(const Tree &tree, uint32_t flags) const noexcept
{

	return Repo::MakeGit<PathSpecMatchList>(git_pathspec_match_tree, tree, flags, pathSpec());
}

std::optional<PathSpecMatchList> PathSpec::matchDiff(const Diff &diff, uint32_t flags) const noexcept
{
	return Repo::MakeGit<PathSpecMatchList>(git_pathspec_match_diff, diff, flags, pathSpec());
}
