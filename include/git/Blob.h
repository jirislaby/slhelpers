// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_BLOB_H
#define GIT_BLOB_H

#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"

namespace SlGit {

class Blob {
	using Holder = SlHelpers::UniqueHolder<git_blob>;

	friend class Repo;
public:
	Blob() = delete;

	const git_oid *id() const { return git_blob_id(blob()); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::string content() const {
		return std::string(static_cast<const char *>(rawcontent()), rawsize());
	}
	std::string_view contentView() const {
		return std::string_view(static_cast<const char *>(rawcontent()), rawsize());
	}

	git_blob *blob() const { return m_blob.get(); }
	operator git_blob *() const { return blob(); }
private:
	git_object_size_t rawsize() const { return git_blob_rawsize(blob()); }
	const void *rawcontent() const { return git_blob_rawcontent(blob()); }

	explicit Blob(git_blob *blob);

	Holder m_blob;
};

}

#endif
