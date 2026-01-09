// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Index.h"
#include "git/Repo.h"
#include "git/StrArray.h"
#include "git/Tree.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_index>::operator()(git_index *idx) const
{
	git_index_free(idx);
}

template<>
void SlHelpers::Deleter<git_index_iterator>::operator()(git_index_iterator *it) const
{
	git_index_iterator_free(it);
}

std::optional<Index> Index::open(const std::filesystem::path &path) noexcept
{
	git_index *index;
	if (git_index_open(&index, path.c_str()))
		return std::nullopt;
	return Index(index);
}

std::optional<Index> Index::create() noexcept
{
	git_index *index;
	if (git_index_new(&index))
		return std::nullopt;
	return Index(index);
}

int Index::readTree(const Tree &tree) const noexcept
{
	return git_index_read_tree(index(), tree);
}

std::optional<Tree> Index::writeTree(const Repo &repo) const noexcept
{
	git_oid oid;
	if (git_index_write_tree(&oid, index()))
		return std::nullopt;
	return repo.treeLookup(oid);
}

int Index::addAll(const std::vector<std::string> &paths, unsigned int flags,
		  const MatchCB *cb) const
{
	return git_index_add_all(index(), StrArray(paths), flags, cb ? matchCB : nullptr,
				 const_cast<void *>(static_cast<const void *>(&cb)));
}

int Index::removeAll(const std::vector<std::string> &paths, const MatchCB *cb) const
{
	return git_index_remove_all(index(), StrArray(paths), cb ? matchCB : nullptr,
				    const_cast<void *>(static_cast<const void *>(&cb)));
}

int Index::updateAll(const std::vector<std::string> &paths, const MatchCB *cb) const
{
	return git_index_update_all(index(), StrArray(paths), cb ? matchCB : nullptr,
				    const_cast<void *>(static_cast<const void *>(&cb)));
}

IndexIterator Index::cbegin() const noexcept
{
	return *Repo::MakeGit<IndexIterator>(git_index_iterator_new, index());
}

int Index::matchCB(const char *path, const char *matched_pathspec, void *payload)
{
	const auto &cb = *static_cast<const MatchCB *>(payload);

	return cb(path, matched_pathspec);
}
