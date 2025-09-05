// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_MISC_H
#define GIT_MISC_H

#include <optional>
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

class Reference {
	using Holder = SlHelpers::UniqueHolder<git_reference>;

	friend class Repo;
public:
	Reference() = delete;

	const git_oid *target() const { return git_reference_target(ref()); }
	const git_oid *targetPeel() const { return git_reference_target_peel(ref()); }
	std::string symbolicTarget() const { return git_reference_symbolic_target(ref()); }
	git_reference_t type() const { return git_reference_type(ref()); }
	std::string name() const { return  git_reference_name(ref()); }

	std::optional<Reference> resolve() const;

	git_reference *ref() const { return m_ref.get(); }
	operator git_reference *() const { return ref(); }
private:
	explicit Reference(git_reference *ref) : m_ref(ref) { }

	Holder m_ref;
};

class Commit;
class Repo;

class RevWalk {
	using Holder = SlHelpers::UniqueHolder<git_revwalk>;

	friend class Repo;
public:
	RevWalk() = delete;

	int pushHead() { return git_revwalk_push_head(m_revWalk); }
	int pushRef(const std::string &ref) { return git_revwalk_push_ref(m_revWalk, ref.c_str()); }
	int pushGlob(const std::string &glob) {
		return git_revwalk_push_glob(m_revWalk, glob.c_str());
	}
	int pushRange(const std::string &range) {
		return git_revwalk_push_range(m_revWalk, range.c_str());
	}

	/* Hiding marks stopping points */
	int hide(const git_oid &oid) { return git_revwalk_hide(m_revWalk, &oid); }
	int hideGlob(const std::string &glob) {
		return git_revwalk_hide_glob(m_revWalk, glob.c_str());
	}

	std::optional<Commit> next(const Repo &repo);

	git_revwalk *revWalk() const { return m_revWalk.get(); }
	operator git_revwalk *() const { return revWalk(); }
private:
	explicit RevWalk(git_revwalk *revWalk) : m_revWalk(revWalk) { }

	Holder m_revWalk;
};

class Signature {
	using Holder = SlHelpers::UniqueHolder<git_signature>;
public:
	Signature() = delete;

	static std::optional<Signature> now(const std::string &name, const std::string &email);

	std::string name() const { return signature()->name; }
	std::string email() const { return signature()->email; }

	git_signature *signature() const { return m_signature.get(); }
	operator git_signature *() const { return signature(); }
private:
	explicit Signature(git_signature *sig) : m_signature(sig) {}

	Holder m_signature;
};

}

#endif
