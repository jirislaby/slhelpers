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

/**
 * @brief Iterator returned from Index::begin()
 */
class IndexIterator {
	using GitTy = git_index_iterator;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Index;
	friend class Repo;
public:
	/// @brief Move to the next entry
	IndexIterator &operator++() {
		increment();
		return *this;
	}
	IndexIterator operator++(int) = delete;

	IndexIterator &operator--() = delete;
	IndexIterator operator--(int) = delete;

	/// @brief Compare with \p other
	bool operator ==(const IndexIterator &other) const noexcept {
		return m_current == other.m_current;
	}
	/// @brief Compare with \p other
	bool operator !=(const IndexIterator &other) const noexcept {
		return !(*this == other);
	}
	/// @brief Get entry from this IndexIterator
	const git_index_entry &operator *() const noexcept { return *m_current; }
	/// @brief Get entry from this IndexIterator
	const git_index_entry *operator ->() const noexcept { return m_current; }

	/// @brief Get path from this IndexIterator (as a string)
	std::string path() const noexcept { return m_current->path; }
	/// @brief Get path from this IndexIterator (as a string_view)
	std::string_view pathSV() const noexcept { return m_current->path; }

	/// @brief Get the stored pointer to libgit2's git_index_entry
	GitTy *iterator() const noexcept { return m_iterator.get(); }
	/// @brief Alias for iterator() -- implicit conversion
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

/**
 * @brief Index is a representation of a git index
 */
class Index {
	using GitTy = git_index;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	/// @brief A callback for addAll(), removeAll(), updateAll()
	using MatchCB = std::function<int(const std::filesystem::path &, const std::string &)>;

	Index() = delete;

	/// @brief Load an index at \p into a new Index
	static std::optional<Index> open(const std::filesystem::path &path) noexcept;
	/// @brief Create a new Index
	static std::optional<Index> create() noexcept;

	/// @brief Read the index from the disk into this Index
	bool read(bool force = true) const noexcept {
		return !Repo::setLastError(git_index_read(index(), force));
	}
	/// @brief Write this Index to disk
	bool write() const noexcept { return !Repo::setLastError(git_index_write(index())); }
	/// @brief Read \p tree into this Index
	bool readTree(const Tree &tree) const noexcept;
	/// @brief Write this Index as a Tree
	std::optional<Tree> writeTree(const Repo &repo) const noexcept;

	/// @brief Get count of entries in this Index
	size_t entrycount() const noexcept { return git_index_entrycount(index()); }
	/// @brief Get an entry on the \p idx-th position
	const git_index_entry *entryByIndex(size_t idx) const noexcept {
		return git_index_get_byindex(index(), idx);
	}
	/// @brief Get entry corresponding to \p path (and \p stage)
	const git_index_entry *entryByPath(const std::filesystem::path &path,
					   git_index_stage_t stage = GIT_INDEX_STAGE_NORMAL) const noexcept {
		return git_index_get_bypath(index(), path.c_str(), stage);
	}

	/// @brief Add \p path to this Index
	bool addByPath(const std::filesystem::path &path) const noexcept {
		return !Repo::setLastError(git_index_add_bypath(index(), path.c_str()));
	}
	/// @brief Remove \p path from this Index
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

	/// @brief Returns true if this index has some conflicts
	bool hasConflicts() const noexcept { return git_index_has_conflicts(index()); }

	/// @brief Get the begin iterator for this Index
	IndexIterator begin() noexcept { return cbegin(); }
	/// @brief Get the end iterator for this Index
	IndexIterator end() noexcept { return IndexIterator(); }

	/// @brief Get the begin iterator for this Index
	IndexIterator cbegin() const noexcept;
	/// @brief Get the end iterator for this Index
	IndexIterator cend() const noexcept { return IndexIterator(); }

	/// @brief Get the stored pointer to libgit2's git_index
	GitTy *index() const noexcept { return m_index.get(); }
	/// @brief Alias for index() -- implicit conversion
	operator GitTy *() const noexcept { return index(); }
private:
	explicit Index(GitTy *index) noexcept : m_index(index) { }

	static int matchCB(const char *path, const char *matched_pathspec, void *payload);

	Holder m_index;
};

}
