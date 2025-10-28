// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_MISC_H
#define SLGIT_MISC_H

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

	const git_oid *target() const noexcept { return git_reference_target(ref()); }
	const git_oid *targetPeel() const noexcept { return git_reference_target_peel(ref()); }
	std::string symbolicTarget() const noexcept { return git_reference_symbolic_target(ref()); }
	git_reference_t type() const noexcept { return git_reference_type(ref()); }
	std::string name() const noexcept { return  git_reference_name(ref()); }

	std::optional<Reference> resolve() const noexcept;

	GitTy *ref() const noexcept { return m_ref.get(); }
	operator GitTy *() const noexcept { return ref(); }
private:
	explicit Reference(GitTy *ref) noexcept : m_ref(ref) { }

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

	int pushHead() const noexcept { return git_revwalk_push_head(revWalk()); }
	int pushRef(const std::string &ref) const noexcept {
		return git_revwalk_push_ref(revWalk(), ref.c_str());
	}
	int pushGlob(const std::string &glob) const noexcept {
		return git_revwalk_push_glob(revWalk(), glob.c_str());
	}
	int pushRange(const std::string &range) const noexcept {
		return git_revwalk_push_range(revWalk(), range.c_str());
	}

	/* Hiding marks stopping points */
	int hide(const git_oid &oid) const noexcept { return git_revwalk_hide(revWalk(), &oid); }
	int hideGlob(const std::string &glob) const noexcept {
		return git_revwalk_hide_glob(revWalk(), glob.c_str());
	}

	std::optional<Commit> next(const Repo &repo) const noexcept;

	GitTy *revWalk() const noexcept { return m_revWalk.get(); }
	operator GitTy *() const noexcept { return revWalk(); }
private:
	explicit RevWalk(GitTy *revWalk) noexcept : m_revWalk(revWalk) { }

	Holder m_revWalk;
};

class Signature {
	using GitTy = git_signature;
	using Holder = SlHelpers::UniqueHolder<GitTy>;
public:
	Signature() = delete;

	static std::optional<Signature> now(const std::string &name, const std::string &email);

	std::string name() const noexcept { return signature()->name; }
	std::string email() const noexcept { return signature()->email; }

	GitTy *signature() const noexcept { return m_signature.get(); }
	operator GitTy *() const noexcept { return signature(); }
private:
	explicit Signature(GitTy *sig) noexcept : m_signature(sig) {}

	Holder m_signature;
};

}

#endif
