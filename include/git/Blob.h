// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_BLOB_H
#define GIT_BLOB_H

#include <string>

#include <git2.h>

#include "Object.h"

namespace SlGit {

class Blob : public TypedObject<git_blob> {
	friend class Repo;
public:
	Blob() = delete;

	std::string content() const {
		return std::string(static_cast<const char *>(rawcontent()), rawsize());
	}
	std::string_view contentView() const {
		return std::string_view(static_cast<const char *>(rawcontent()), rawsize());
	}

	git_blob *blob() const { return typed(); }
private:
	git_object_size_t rawsize() const { return git_blob_rawsize(blob()); }
	const void *rawcontent() const { return git_blob_rawcontent(blob()); }

	explicit Blob(git_blob *blob);
};

}

#endif
