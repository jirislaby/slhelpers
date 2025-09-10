// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_INDEX_H
#define GIT_INDEX_H

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"

namespace SlGit {

class Repo;
class Tree;

class Index {
	using GitTy = git_index;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	using MatchCB = std::function<int(const std::filesystem::path &, const std::string &)>;

	Index() = delete;

	static std::optional<Index> open(const std::filesystem::path &path);
	static std::optional<Index> create();

	int read(bool force = true) { return git_index_read(index(), force); }
	int write() { return git_index_write(index()); }
	int readTree(const Tree &tree);
	std::optional<Tree> writeTree(const Repo &repo);

	const git_index_entry *entryByIndex(size_t idx) const {
		return git_index_get_byindex(index(), idx);
	}
	const git_index_entry *entryByPath(const std::filesystem::path &path,
					   git_index_stage_t stage = GIT_INDEX_STAGE_NORMAL) const {
		return git_index_get_bypath(index(), path.c_str(), stage);
	}

	int addByPath(const std::filesystem::path &path) {
		return git_index_add_bypath(index(), path.c_str());
	}
	int removeByPath(const std::filesystem::path &path) {
		return git_index_remove_bypath(index(), path.c_str());
	}
	int addAll(const std::vector<std::string> &paths, unsigned int flags, const MatchCB &cb);
	int removeAll(const std::vector<std::string> &paths, const MatchCB &cb);
	int updateAll(const std::vector<std::string> &paths, const MatchCB &cb);

	bool hasConflicts() const { return git_index_has_conflicts(index()); }

	GitTy *index() const { return m_index.get(); }
	operator GitTy *() const { return index(); }
private:
	explicit Index(GitTy *index) : m_index(index) { }

	static int matchCB(const char *path, const char *matched_pathspec, void *payload);

	Holder m_index;
};

}

#endif
