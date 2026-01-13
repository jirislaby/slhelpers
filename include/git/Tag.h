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

/**
 * @brief Tag is a representation of a git tag
 */
class Tag : public TypedObject<git_tag> {
	using GitTy = git_tag;

	friend class Repo;
public:
	Tag() = delete;

	/// @brief Get the OID of the target of this Tag
	const git_oid *targetId() const noexcept { return git_tag_target_id(tag()); }
	/// @brief Get the SHA of the target of this Tag
	std::string targetIdStr() const noexcept { return  Helpers::oidToStr(*targetId()); }
	/// @brief Get the type of the target of this Tag (GIT_OBJECT_COMMIT, ...)
	git_object_t targetType() const noexcept { return git_tag_target_type(tag()); }

	/// @brief Get the name of this Tag (the tag proper)
	std::string name() const noexcept { return git_tag_name(tag()); }
	/// @brief Get the tagger of this Tag
	const git_signature *tagger() const noexcept { return git_tag_tagger(tag()); }
	/// @brief Get the tag message of this Tag
	std::string message() const noexcept { return git_tag_message(tag()); }

	/// @brief Peel this Tag until the underlaying object is found and return that
	std::variant<std::monostate, Commit, Tree, Blob> peel() const noexcept;

	/// @brief Get the stored pointer to libgit2's git_tag
	GitTy *tag() const noexcept { return typed(); }
private:
	explicit Tag(const Repo &repo, GitTy *tag) noexcept;
};

}
