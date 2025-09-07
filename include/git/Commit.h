// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_COMMIT_H
#define GIT_COMMIT_H

#include <optional>
#include <string>

#include <git2.h>

#include "Helpers.h"
#include "Object.h"

namespace SlGit {

class Repo;
class Tree;

class Commit : public TypedObject<git_commit> {
	friend class Repo;
public:
	Commit() = delete;

	std::optional<Commit> parent(const Commit &ofCommit, unsigned int nth);
	std::optional<Commit> ancestor(const Commit &ofCommit, unsigned int nth);

	std::optional<Tree> tree() const;

	const git_oid *treeId() const { return git_commit_tree_id(commit()); }
	std::string treeIdStr() const { return Helpers::oidToStr(*treeId()); }
	std::string messageEncoding() const { return git_commit_message_encoding(commit()); }
	std::string message() const { return git_commit_message(commit()); }
	std::string summary() const { return git_commit_summary(commit()); }
	git_time_t time() const { return git_commit_time(commit()); }
	int timeOffset() const { return git_commit_time_offset(commit()); }
	const git_signature *committer() const { return git_commit_committer(commit()); }
	const git_signature *author() const { return git_commit_author(commit()); }
	std::string rawHeader() const { return git_commit_raw_header(commit()); }

	unsigned int parentCount() const { return git_commit_parentcount(commit()); }
	const git_oid *parentId(unsigned int nth) const { return git_commit_parent_id(commit(), nth); }

	std::optional<std::string> catFile(const Repo &repo, const std::string &file) const;

	git_commit *commit() const { return typed(); }
	operator git_commit *() const { return commit(); }
private:
	explicit Commit(git_commit *commit) : TypedObject(commit) { }
};

}

#endif
