// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_BLOB_H
#define SLGIT_BLOB_H

#include <string>

#include <git2.h>

#include "Object.h"

namespace SlGit {

class Blob : public TypedObject<git_blob> {
	using GitTy = git_blob;

	friend class Repo;
public:
	Blob() = delete;

	std::string content() const noexcept {
		return std::string(static_cast<const char *>(rawcontent()), rawsize());
	}
	std::string_view contentView() const noexcept {
		return std::string_view(static_cast<const char *>(rawcontent()), rawsize());
	}

	GitTy *blob() const noexcept { return typed(); }
private:
	git_object_size_t rawsize() const noexcept { return git_blob_rawsize(blob()); }
	const void *rawcontent() const noexcept { return git_blob_rawcontent(blob()); }

	explicit Blob(const Repo &repo, GitTy *blob) noexcept;
};

}

#endif
