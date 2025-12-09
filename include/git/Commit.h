// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_COMMIT_H
#define SLGIT_COMMIT_H

#include <optional>
#include <string>

#include <git2.h>

#include "Helpers.h"
#include "Object.h"

namespace SlGit {

class Repo;
class Tree;

class Commit : public TypedObject<git_commit> {
	using GitTy = git_commit;

	friend class Repo;
public:
	Commit() = delete;

	std::optional<Commit> parent(unsigned int nth = 0) const noexcept;
	std::optional<Commit> ancestor(unsigned int nth = 0) const noexcept;

	std::optional<Tree> tree() const noexcept;

	const git_oid *treeId() const noexcept { return git_commit_tree_id(commit()); }
	std::string treeIdStr() const noexcept { return Helpers::oidToStr(*treeId()); }
	std::string messageEncoding() const noexcept {
		return git_commit_message_encoding(commit());
	}
	std::string message() const noexcept { return git_commit_message(commit()); }
	std::string summary() const noexcept { return git_commit_summary(commit()); }
	git_time_t time() const noexcept { return git_commit_time(commit()); }
	int timeOffset() const noexcept { return git_commit_time_offset(commit()); }
	const git_signature *committer() const noexcept { return git_commit_committer(commit()); }
	const git_signature *author() const noexcept { return git_commit_author(commit()); }
	std::string rawHeader() const noexcept { return git_commit_raw_header(commit()); }

	unsigned int parentCount() const noexcept { return git_commit_parentcount(commit()); }
	const git_oid *parentId(unsigned int nth) const noexcept {
		return git_commit_parent_id(commit(), nth);
	}

	std::optional<std::string> catFile(const std::string &file) const noexcept;

	GitTy *commit() const noexcept { return typed(); }
	operator GitTy *() const noexcept { return commit(); }
private:
	explicit Commit(const Repo &repo, GitTy *commit) noexcept :
		TypedObject(repo, commit) { }
};

}

#endif
