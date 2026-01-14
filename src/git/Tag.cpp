// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "git/Tag.h"
#include "helpers/PtrStore.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_tag>::operator()(git_tag *tag) const
{
	git_tag_free(tag);
}

using ObjectStore = SlHelpers::PtrStore<git_object,
	decltype([](git_object *obj){ git_object_free(obj); })>;

std::variant<std::monostate, Commit, Tree, Blob> Tag::peel() const noexcept
{
	ObjectStore obj;
	if (git_tag_peel(obj.ptr(), tag()))
		return std::monostate();
	switch (git_object_type(*obj)) {
	case GIT_OBJECT_COMMIT:
		return Commit(repo(), reinterpret_cast<git_commit *>(obj.release()));
	case GIT_OBJECT_TREE:
		return Tree(repo(), reinterpret_cast<git_tree *>(obj.release()));
	case GIT_OBJECT_BLOB:
		return Blob(repo(), reinterpret_cast<git_blob *>(obj.release()));
	default:
		return std::monostate();
	}
}

Tag::Tag(const Repo &repo, GitTy *tag) noexcept : TypedObject(repo, tag)
{
}
