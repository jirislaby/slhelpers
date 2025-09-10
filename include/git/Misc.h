// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_MISC_H
#define GIT_MISC_H

#include <optional>
#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

namespace SlGit {

class Reference {
	using GitTy = git_reference;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	Reference() = delete;

	const git_oid *target() const { return git_reference_target(ref()); }
	const git_oid *targetPeel() const { return git_reference_target_peel(ref()); }
	std::string symbolicTarget() const { return git_reference_symbolic_target(ref()); }
	git_reference_t type() const { return git_reference_type(ref()); }
	std::string name() const { return  git_reference_name(ref()); }

	std::optional<Reference> resolve() const;

	GitTy *ref() const { return m_ref.get(); }
	operator GitTy *() const { return ref(); }
private:
	explicit Reference(GitTy *ref) : m_ref(ref) { }

	Holder m_ref;
};

class Commit;
class Repo;

class RevWalk {
	using GitTy = git_revwalk;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

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

	GitTy *revWalk() const { return m_revWalk.get(); }
	operator GitTy *() const { return revWalk(); }
private:
	explicit RevWalk(GitTy *revWalk) : m_revWalk(revWalk) { }

	Holder m_revWalk;
};

class Signature {
	using GitTy = git_signature;
	using Holder = SlHelpers::UniqueHolder<GitTy>;
public:
	Signature() = delete;

	static std::optional<Signature> now(const std::string &name, const std::string &email);

	std::string name() const { return signature()->name; }
	std::string email() const { return signature()->email; }

	GitTy *signature() const { return m_signature.get(); }
	operator GitTy *() const { return signature(); }
private:
	explicit Signature(GitTy *sig) : m_signature(sig) {}

	Holder m_signature;
};

}

#endif
