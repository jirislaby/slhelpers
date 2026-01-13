// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string>

#include <git2.h>

#include "Helpers.h"
#include "Object.h"

namespace SlGit {

class Repo;
class Tree;

/**
 * @brief Commit is a representation of a git commit
 */
class Commit : public TypedObject<git_commit> {
	using GitTy = git_commit;

	friend class Repo;
public:
	Commit() = delete;

	/// @brief Get the \p nth parent of this Commit
	std::optional<Commit> parent(unsigned int nth = 0) const noexcept;
	/// @brief Get the \p nth generation ancestor of this Commit
	std::optional<Commit> ancestor(unsigned int nth = 0) const noexcept;

	/// @brief Get the Tree of this Commit
	std::optional<Tree> tree() const noexcept;

	/// @brief Get the OID of the tree of this Commit
	const git_oid *treeId() const noexcept { return git_commit_tree_id(commit()); }
	/// @brief Get the SHA of the tree of this Commit
	std::string treeIdStr() const noexcept { return Helpers::oidToStr(*treeId()); }
	/// @brief Get the commit message encoding of this Commit
	std::string messageEncoding() const noexcept {
		return git_commit_message_encoding(commit());
	}
	/// @brief Get the commit message of this Commit
	std::string message() const noexcept { return git_commit_message(commit()); }
	/// @brief Get the summary line of this Commit
	std::string summary() const noexcept { return git_commit_summary(commit()); }
	/// @brief Get the time of this Commit
	git_time_t time() const noexcept { return git_commit_time(commit()); }
	/// @brief Get the timezone offset of this Commit
	int timeOffset() const noexcept { return git_commit_time_offset(commit()); }
	/// @brief Get the committer of this Commit
	const git_signature *committer() const noexcept { return git_commit_committer(commit()); }
	/// @brief Get the author of this Commit
	const git_signature *author() const noexcept { return git_commit_author(commit()); }
	/// @brief Get a raw Commit header
	std::string rawHeader() const noexcept { return git_commit_raw_header(commit()); }

	/// @brief Get count of parents
	unsigned int parentCount() const noexcept { return git_commit_parentcount(commit()); }
	/// @brief Get OID of the \p nth parent
	const git_oid *parentId(unsigned int nth) const noexcept {
		return git_commit_parent_id(commit(), nth);
	}

	/// @brief Cat a \p file in this Commit's tree
	std::optional<std::string> catFile(const std::string &file) const noexcept;

	/// @brief Get the stored pointer to libgit2's git_commit
	GitTy *commit() const noexcept { return typed(); }
	/// @brief Alias for commit() -- implicit conversion
	operator GitTy *() const noexcept { return commit(); }
private:
	friend class Tag;
	explicit Commit(const Repo &repo, GitTy *commit) noexcept :
		TypedObject(repo, commit) { }
};

}
