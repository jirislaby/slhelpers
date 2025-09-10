// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_TAG_H
#define GIT_TAG_H

#include <string>

#include <git2.h>

#include "Helpers.h"
#include "Object.h"

namespace SlGit {

class Repo;

class Tag : public TypedObject<git_tag> {
	using GitTy = git_tag;

	friend class Repo;
public:
	Tag() = delete;

	const git_oid *targetId() const noexcept { return git_tag_target_id(tag()); }
	std::string targetIdStr() const noexcept { return  Helpers::oidToStr(*targetId()); }
	git_object_t targetType() const noexcept { return git_tag_target_type(tag()); }

	std::string name() const noexcept { return git_tag_name(tag()); }
	const git_signature *tagger() const noexcept { return git_tag_tagger(tag()); }
	std::string message() const noexcept { return git_tag_message(tag()); }

	GitTy *tag() const noexcept { return typed(); }
private:
	explicit Tag(GitTy *tag) noexcept;
};

}

#endif
