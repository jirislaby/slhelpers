// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>

#include <git2.h>
#include <variant>

#include "Helpers.h"
#include "Object.h"

namespace SlGit {

class Blob;
class Commit;
class Repo;
class Tree;

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

	std::variant<std::monostate, Commit, Tree, Blob> peel() const noexcept;

	GitTy *tag() const noexcept { return typed(); }
private:
	explicit Tag(const Repo &repo, GitTy *tag) noexcept;
};

}
