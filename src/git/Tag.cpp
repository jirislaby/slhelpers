// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Repo.h"
#include "git/Tag.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_tag>::operator()(git_tag *tag) const
{
	git_tag_free(tag);
}

Tag::Tag(git_tag *tag) : TypedObject(tag)
{
}
