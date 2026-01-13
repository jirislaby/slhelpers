// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string>

#include <git2.h>

#include "../helpers/Unique.h"
#include "Repo.h"

namespace SlGit {

/**
 * @brief Reference is a representation of a git reference
 */
class Reference {
	using GitTy = git_reference;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	Reference() = delete;

	/// @brief Get the target of a Reference
	const git_oid *target() const noexcept { return git_reference_target(ref()); }
	/// @brief Get the peeled target of a Reference
	const git_oid *targetPeel() const noexcept { return git_reference_target_peel(ref()); }
	/// @brief Get the target of a symbolic Reference
	std::string symbolicTarget() const noexcept { return git_reference_symbolic_target(ref()); }
	/// @brief Get type of this Reference (\c GIT_REFERENCE_DIRECT, \c GIT_REFERENCE_SYMBOLIC)
	git_reference_t type() const noexcept { return git_reference_type(ref()); }
	/// @brief Get name of this Reference
	std::string name() const noexcept { return  git_reference_name(ref()); }

	/// @brief Resolve a symbolic reference to a direct reference
	std::optional<Reference> resolve() const noexcept;

	/// @brief Get the stored pointer to libgit2's git_reference
	GitTy *ref() const noexcept { return m_ref.get(); }
	/// @brief Alias for ref() -- implicit conversion
	operator GitTy *() const noexcept { return ref(); }
private:
	explicit Reference(GitTy *ref) noexcept : m_ref(ref) { }

	Holder m_ref;
};

class Commit;

/**
 * @brief RevWalk is a representation of a git revwalk
 */
class RevWalk {
	using GitTy = git_revwalk;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	RevWalk() = delete;

	/// @brief Add one OID to this RevWalk
	bool push(const git_oid &oid) const noexcept {
		return !Repo::setLastError(git_revwalk_push(revWalk(), &oid));
	}
	/// @brief Add one commit SHA to this RevWalk
	bool push(const std::string &id) const noexcept;
	/// @brief Add HEAD to this RevWalk
	bool pushHead() const noexcept {
		return !Repo::setLastError(git_revwalk_push_head(revWalk()));
	}
	/// @brief Add a commit referenced by \p ref to this RevWalk
	bool pushRef(const std::string &ref) const noexcept {
		return !Repo::setLastError(git_revwalk_push_ref(revWalk(), ref.c_str()));
	}
	/// @brief Add all references matching \p glob
	bool pushGlob(const std::string &glob) const noexcept {
		return !Repo::setLastError(git_revwalk_push_glob(revWalk(), glob.c_str()));
	}
	/// @brief Add all references matching \p range (commit1..commit2)
	bool pushRange(const std::string &range) const noexcept {
		return !Repo::setLastError(git_revwalk_push_range(revWalk(), range.c_str()));
	}

	/// @brief Stop this RevWalk at \p oid
	bool hide(const git_oid &oid) const noexcept {
		return !Repo::setLastError(git_revwalk_hide(revWalk(), &oid));
	}
	/// @brief Stop this RevWalk at commit \p id
	bool hide(const std::string &id) const noexcept;
	/// @brief Stop this RevWalk at references matching \p glob
	bool hideGlob(const std::string &glob) const noexcept {
		return !Repo::setLastError(git_revwalk_hide_glob(revWalk(), glob.c_str()));
	}

	/// @brief Set \p mode as sorting mode (\c GIT_SORT_NONE, \c GIT_SORT_TOPOLOGICAL, ...)
	bool sorting(unsigned int mode) const noexcept {
		return !Repo::setLastError(git_revwalk_sorting(revWalk(), mode));
	}

	/// @brief Get next Commit in this RevWalk, or nullopt if there are no more
	std::optional<Commit> next() const noexcept;

	/// @brief Get the stored pointer to libgit2's git_revwalk
	GitTy *revWalk() const noexcept { return m_revWalk.get(); }
	/// @brief Alias for revWalk() -- implicit conversion
	operator GitTy *() const noexcept { return revWalk(); }
private:
	explicit RevWalk(const Repo &repo, GitTy *revWalk) noexcept :
		m_repo(repo), m_revWalk(revWalk) { }

	const Repo &m_repo;
	Holder m_revWalk;
};

/**
 * @brief Signature is a representation of a git signature
 */
class Signature {
	using GitTy = git_signature;
	using Holder = SlHelpers::UniqueHolder<GitTy>;
public:
	Signature() = delete;

	/// @brief Create a new Signature with \p name and \p email
	static std::optional<Signature> now(const std::string &name, const std::string &email);

	/// @brief Get the name in this Signature
	std::string name() const noexcept { return signature()->name; }
	/// @brief Get the email in this Signature
	std::string email() const noexcept { return signature()->email; }

	/// @brief Get the stored pointer to libgit2's git_signature
	GitTy *signature() const noexcept { return m_signature.get(); }
	/// @brief Alias for signature() -- implicit conversion
	operator GitTy *() const noexcept { return signature(); }
private:
	explicit Signature(GitTy *sig) noexcept : m_signature(sig) {}

	Holder m_signature;
};

}
