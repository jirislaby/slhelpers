// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>

#include <git2.h>

#include "Object.h"

namespace SlGit {

/**
 * @brief Blob is a representation of a git blob
 */
class Blob : public TypedObject<git_blob> {
	using GitTy = git_blob;

	friend class Repo;
public:
	Blob() = delete;

	/// @brief Get the content of this Blob (as a string)
	std::string content() const noexcept {
		return std::string(static_cast<const char *>(rawcontent()), rawsize());
	}
	/// @brief Get the content of this Blob (as a string_view)
	std::string_view contentView() const noexcept {
		return std::string_view(static_cast<const char *>(rawcontent()), rawsize());
	}

	/// @brief Get the stored pointer to libgit2's git_blob
	GitTy *blob() const noexcept { return typed(); }
private:
	git_object_size_t rawsize() const noexcept { return git_blob_rawsize(blob()); }
	const void *rawcontent() const noexcept { return git_blob_rawcontent(blob()); }

	friend class Tag;
	explicit Blob(const Repo &repo, GitTy *blob) noexcept;
};

}
