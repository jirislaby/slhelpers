// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "git/Tag.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_tag>::operator()(git_tag *tag) const
{
	git_tag_free(tag);
}

std::variant<std::monostate, Commit, Tree, Blob> Tag::peel() const noexcept
{
	git_object *obj;
	if (git_tag_peel(&obj, tag()))
		return std::monostate();
	switch (git_object_type(obj)) {
	case GIT_OBJECT_COMMIT:
		return Commit(repo(), reinterpret_cast<git_commit *>(obj));
	case GIT_OBJECT_TREE:
		return Tree(repo(), reinterpret_cast<git_tree *>(obj));
	case GIT_OBJECT_BLOB:
		return Blob(repo(), reinterpret_cast<git_blob *>(obj));
	default:
		return std::monostate();
	}
}

Tag::Tag(const Repo &repo, GitTy *tag) noexcept : TypedObject(repo, tag)
{
}
