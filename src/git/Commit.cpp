// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_commit>::operator()(git_commit *commit) const
{
	git_commit_free(commit);
}

std::optional<Commit> Commit::parent(const Commit &ofCommit, unsigned int nth)
{
	git_commit *commit;
	if (git_commit_parent(&commit, ofCommit, nth))
		return std::nullopt;
	return Commit(commit);
}

std::optional<Commit> Commit::ancestor(const Commit &ofCommit, unsigned int nth)
{
	git_commit *commit;
	if (git_commit_nth_gen_ancestor(&commit, ofCommit, nth))
		return std::nullopt;
	return Commit(commit);
}

std::optional<Tree> Commit::tree() const
{
	git_tree *tree;
	if (git_commit_tree(&tree, commit()))
		return std::nullopt;
	return Tree(tree);
}

std::optional<std::string> Commit::catFile(const Repo &repo, const std::string &file) const
{
	if (auto t = tree())
		return t->catFile(repo, file);
	return std::nullopt;
}
