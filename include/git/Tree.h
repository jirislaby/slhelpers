// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"
#include "Object.h"
#include "Repo.h"

namespace SlGit {

class Blob;
class Commit;
class TreeEntry;

/**
 * @brief Tree is a representation of a git tree
 */
class Tree : public TypedObject<git_tree> {
	using GitTy = git_tree;

	friend class Commit;
	friend class Repo;
public:
	/// @brief A callback for walk()
	using WalkCallback = const std::function<int(const std::string &root,
		const TreeEntry &entry)>;
	Tree() = delete;

	/// @brief Get count of entries in this Tree
	size_t entryCount() const noexcept { return git_tree_entrycount(tree()); }

	/// @brief Walk this Tree and call \p CB for every entry
	bool walk(const WalkCallback &CB, const git_treewalk_mode &mode = GIT_TREEWALK_PRE) const;

	/// @brief Get an entry corresponding to \p path
	std::optional<TreeEntry> treeEntryByPath(const std::string &path) const noexcept;
	/// @brief Get an entry on the \p idx-th position
	TreeEntry treeEntryByIndex(size_t idx) const noexcept;

	/// @brief Cat a \p file in this Tree
	std::optional<std::string> catFile(const std::string &file) const noexcept;

	/// @brief Get the stored pointer to libgit2's git_tree
	GitTy *tree() const noexcept { return typed(); }
private:
	static int walkCB(const char *root, const git_tree_entry *entry, void *payload);

	friend class Tag;
	explicit Tree(const Repo &repo, GitTy *tree) noexcept : TypedObject(repo, tree) {}
};

/**
 * @brief The TreeEntry represents a libgit2's tree builder
 */
class TreeBuilder {
	using GitTy = git_treebuilder;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	TreeBuilder() = delete;

	/// @brief Add \p file to this TreeBuilder, having \p blob as a content
	bool insert(const std::filesystem::path &file, const Blob &blob) const noexcept;
	/// @brief Remove \p file from this TreeBuilder
	bool remove(const std::filesystem::path &file) const noexcept {
		return !Repo::setLastError(git_treebuilder_remove(treeBuilder(), file.c_str()));
	}
	/// @brief Wipe out this TreeBuilder
	bool clear() const noexcept {
		return !Repo::setLastError(git_treebuilder_clear(treeBuilder()));
	}

	/// @brief Write this TreeBuilder to the repo and return the built Tree
	std::optional<Tree> write(const Repo &repo) const noexcept;

	/// @brief Get count of entries in this TreeBuilder
	size_t entryCount() const noexcept { return git_treebuilder_entrycount(treeBuilder()); }

	/// @brief Get a tree entry for \p file in this TreeBuilder
	const git_tree_entry *get(const std::filesystem::path &file) const noexcept {
		return git_treebuilder_get(treeBuilder(), file.c_str());
	}

	/// @brief Get the stored pointer to libgit2's git_treebuilder
	GitTy *treeBuilder() const noexcept { return m_treeBuilder.get(); }
	/// @brief Alias for treeEntry() -- implicit conversion
	operator GitTy *() const noexcept { return treeBuilder(); }
private:
	explicit TreeBuilder(GitTy *TB) noexcept : m_treeBuilder(TB) { }

	Holder m_treeBuilder;
};

/**
 * @brief The TreeEntry represents one git tree entry
 */
class TreeEntry {
	using GitTy = git_tree_entry;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Tree;
public:
	TreeEntry() = delete;

	/// @brief Get OID (SHA) of this Object
	const git_oid *id() const noexcept { return git_tree_entry_id(treeEntry()); }
	/// @brief Get OID (SHA) of this Object -- as a string
	std::string idStr() const noexcept { return Helpers::oidToStr(*id()); }

	/// @brief Get name of this TreeEntry
	std::string name() const noexcept { return git_tree_entry_name(treeEntry()); }
	/// @brief Get type of this TreeEntry (GIT_OBJECT_TREE, GIT_OBJECT_BLOB, ...)
	git_object_t type() const noexcept { return git_tree_entry_type(treeEntry()); }
	/// @brief Get permissions of this TreeEntry
	git_filemode_t filemode() const noexcept { return git_tree_entry_filemode(treeEntry()); }

	/// @brief Cat this TreeEntry
	std::optional<std::string> catFile(const Repo &repo) const noexcept;

	/// @brief Get the stored pointer to libgit2's git_tree_entry
	GitTy *treeEntry() const noexcept { return m_treeEntry.get(); }
	/// @brief Alias for treeEntry()
	operator GitTy *() const noexcept { return treeEntry(); }
private:
	explicit TreeEntry(GitTy *entry, bool free = true) noexcept : m_treeEntry(entry, free) { }
	explicit TreeEntry(const GitTy *entry) noexcept :
		m_treeEntry(const_cast<GitTy *>(entry), false) { }

	Holder m_treeEntry;
};

}
