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

std::optional<Commit> Commit::parent(unsigned int nth) const noexcept
{
	git_commit *parent;
	if (git_commit_parent(&parent, commit(), nth))
		return std::nullopt;
	return Commit(*m_repo, parent);
}

std::optional<Commit> Commit::ancestor(unsigned int nth) const noexcept
{
	git_commit *ancestor;
	if (git_commit_nth_gen_ancestor(&ancestor, commit(), nth))
		return std::nullopt;
	return Commit(*m_repo, ancestor);
}

std::optional<Tree> Commit::tree() const noexcept
{
	git_tree *tree;
	if (git_commit_tree(&tree, commit()))
		return std::nullopt;
	return Tree(*m_repo, tree);
}

std::optional<std::string> Commit::catFile(const std::string &file) const noexcept
{
	if (auto t = tree())
		return t->catFile(file);
	return std::nullopt;
}
