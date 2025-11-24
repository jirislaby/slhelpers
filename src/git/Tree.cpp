// SPDX-License-Identifier: GPL-2.0-only

#include <filesystem>

#include <git2.h>
#include <optional>

#include "git/Blob.h"
#include "git/Repo.h"
#include "git/Tree.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_tree>::operator()(git_tree *tree) const
{
	git_tree_free(tree);
}

template<>
void SlHelpers::Deleter<git_treebuilder>::operator()(git_treebuilder *TB) const
{
	git_treebuilder_free(TB);
}

template<>
void SlHelpers::Deleter<git_tree_entry>::operator()(git_tree_entry *TE) const
{
	if (m_free)
		git_tree_entry_free(TE);
}

int Tree::walk(const WalkCallback &CB, const git_treewalk_mode &mode) const
{
	return git_tree_walk(tree(), mode, walkCB,
			     const_cast<void *>(static_cast<const void *>(&CB)));
}

std::optional<TreeEntry> Tree::treeEntryByPath(const std::string &path) const noexcept
{
	git_tree_entry *TE;
	if (git_tree_entry_bypath(&TE, tree(), path.c_str()))
		return std::nullopt;
	return TreeEntry(TE);
}

TreeEntry Tree::treeEntryByIndex(size_t idx) const noexcept
{
	return TreeEntry(git_tree_entry_byindex(tree(), idx));
}

std::optional<std::string> Tree::catFile(const std::string &file) const noexcept
{
	if (auto treeEntry = treeEntryByPath(file))
		return treeEntry->catFile(m_repo);

	return std::nullopt;
}

int Tree::walkCB(const char *root, const git_tree_entry *entry, void *payload)
{
	const auto CB = *static_cast<Tree::WalkCallback *>(payload);

	return CB(root, std::move(TreeEntry(entry)));
}

int TreeBuilder::insert(const std::filesystem::path &file, const Blob &blob) const noexcept
{
	return git_treebuilder_insert(nullptr, treeBuilder(), file.c_str(), blob.id(),
				      GIT_FILEMODE_BLOB);
}

std::optional<Tree> TreeBuilder::write(const Repo &repo) const noexcept
{
	git_oid oid;
	auto ret = git_treebuilder_write(&oid, treeBuilder());
	if (ret)
		return std::nullopt;
	return repo.treeLookup(oid);
}

std::optional<std::string> TreeEntry::catFile(const Repo &repo) const noexcept
{
	if (type() != GIT_OBJECT_BLOB)
		return std::nullopt;

	if (auto blob = repo.blobLookup(*this))
		return blob->content();

	return std::nullopt;
}
