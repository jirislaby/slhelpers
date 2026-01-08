// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"
#include "Repo.h"

namespace SlGit {

class Tree;

class IndexIterator {
	using GitTy = git_index_iterator;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Index;
	friend class Repo;
public:
	IndexIterator &operator++() {
		increment();
		return *this;
	}
	IndexIterator operator++(int) = delete;

	IndexIterator &operator--() = delete;
	IndexIterator operator--(int) = delete;

	bool operator ==(const IndexIterator &other) const noexcept {
		return m_current == other.m_current;
	}
	bool operator !=(const IndexIterator &other) const noexcept {
		return !(*this == other);
	}
	const git_index_entry &operator *() const noexcept { return *m_current; }
	const git_index_entry *operator ->() const noexcept { return m_current; }

	std::string path() const noexcept { return m_current->path; }
	std::string_view pathSV() const noexcept { return m_current->path; }

	GitTy *iterator() const noexcept { return m_iterator.get(); }
	operator GitTy *() const noexcept { return iterator(); }
private:
	IndexIterator() noexcept : m_current(nullptr) {}
	explicit IndexIterator(GitTy *iterator) noexcept : m_iterator(iterator) { increment(); }

	void increment() {
		if (git_index_iterator_next(&m_current, iterator()))
			m_current = nullptr;
	}

	Holder m_iterator;
	const git_index_entry *m_current;
};

class Index {
	using GitTy = git_index;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	using MatchCB = std::function<int(const std::filesystem::path &, const std::string &)>;

	Index() = delete;

	static std::optional<Index> open(const std::filesystem::path &path) noexcept;
	static std::optional<Index> create() noexcept;

	bool read(bool force = true) const noexcept {
		return !Repo::setLastError(git_index_read(index(), force));
	}
	bool write() const noexcept { return !Repo::setLastError(git_index_write(index())); }
	bool readTree(const Tree &tree) const noexcept;
	std::optional<Tree> writeTree(const Repo &repo) const noexcept;

	size_t entrycount() const noexcept { return git_index_entrycount(index()); }
	const git_index_entry *entryByIndex(size_t idx) const noexcept {
		return git_index_get_byindex(index(), idx);
	}
	const git_index_entry *entryByPath(const std::filesystem::path &path,
					   git_index_stage_t stage = GIT_INDEX_STAGE_NORMAL) const noexcept {
		return git_index_get_bypath(index(), path.c_str(), stage);
	}

	bool addByPath(const std::filesystem::path &path) const noexcept {
		return !Repo::setLastError(git_index_add_bypath(index(), path.c_str()));
	}
	bool removeByPath(const std::filesystem::path &path) const noexcept {
		return !Repo::setLastError(git_index_remove_bypath(index(), path.c_str()));
	}

	/**
	 * @brief Add all \p paths into this Index
	 * @param paths Paths to add
	 * @param flags Combination of git_index_add_option_t
	 * @param cb Callback for each added/updated path (or nullptr)
	 * @return 0 on success, negative callback return value, or error code.
	 */
	int addAll(const std::vector<std::string> &paths, unsigned int flags,
		   const MatchCB *cb = nullptr) const;
	/**
	 * @brief Remove all \p paths from this Index
	 * @param paths Paths to remove
	 * @param cb Callback for each removed path (or nullptr)
	 * @return 0 on success, negative callback return value, or error code.
	 */
	int removeAll(const std::vector<std::string> &paths, const MatchCB *cb = nullptr) const;
	/**
	 * @brief Update all \p paths in this Index
	 * @param paths Paths to update
	 * @param cb Callback for each updated path (or nullptr)
	 * @return 0 on success, negative callback return value, or error code.
	 */
	int updateAll(const std::vector<std::string> &paths, const MatchCB *cb = nullptr) const;

	bool hasConflicts() const noexcept { return git_index_has_conflicts(index()); }

	IndexIterator begin() noexcept { return cbegin(); }
	IndexIterator end() noexcept { return IndexIterator(); }

	IndexIterator cbegin() const noexcept;
	IndexIterator cend() const noexcept { return IndexIterator(); }

	GitTy *index() const noexcept { return m_index.get(); }
	operator GitTy *() const noexcept { return index(); }
private:
	explicit Index(GitTy *index) noexcept : m_index(index) { }

	static int matchCB(const char *path, const char *matched_pathspec, void *payload);

	Holder m_index;
};

}
