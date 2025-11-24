// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_TREE_H
#define SLGIT_TREE_H

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"
#include "Object.h"

namespace SlGit {

class Blob;
class Commit;
class Repo;
class TreeEntry;

class Tree : public TypedObject<git_tree> {
	using GitTy = git_tree;

	friend class Commit;
	friend class Repo;
public:
	using WalkCallback = const std::function<int(const std::string &root,
		const TreeEntry &entry)>;
	Tree() = delete;

	size_t entryCount() const noexcept { return git_tree_entrycount(tree()); }

	int walk(const WalkCallback &CB, const git_treewalk_mode &mode = GIT_TREEWALK_PRE) const;

	std::optional<TreeEntry> treeEntryByPath(const std::string &path) const noexcept;
	TreeEntry treeEntryByIndex(size_t idx) const noexcept;

	std::optional<std::string> catFile(const std::string &file) const noexcept;

	GitTy *tree() const noexcept { return typed(); }
private:
	static int walkCB(const char *root, const git_tree_entry *entry, void *payload);

	explicit Tree(const Repo &repo, GitTy *tree) noexcept : TypedObject(tree), m_repo(repo) {}

	const Repo &m_repo;
};

class TreeBuilder {
	using GitTy = git_treebuilder;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	TreeBuilder() = delete;

	int insert(const std::filesystem::path &file, const Blob &blob) const noexcept;
	int remove(const std::filesystem::path &file) const noexcept {
		return git_treebuilder_remove(treeBuilder(), file.c_str());
	}
	int clear() const noexcept { return git_treebuilder_clear(treeBuilder()); }

	std::optional<Tree> write(const Repo &repo) const noexcept;

	size_t entryCount() const noexcept { return git_treebuilder_entrycount(treeBuilder()); }

	const git_tree_entry *get(const std::filesystem::path &file) const noexcept {
		return git_treebuilder_get(treeBuilder(), file.c_str());
	}

	GitTy *treeBuilder() const noexcept { return m_treeBuilder.get(); }
	operator GitTy *() const noexcept { return treeBuilder(); }
private:
	explicit TreeBuilder(GitTy *TB) noexcept : m_treeBuilder(TB) { }

	Holder m_treeBuilder;
};

class TreeEntry {
	using GitTy = git_tree_entry;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Tree;
public:
	TreeEntry() = delete;

	const git_oid *id() const noexcept { return git_tree_entry_id(treeEntry()); }
	std::string idStr() const noexcept { return Helpers::oidToStr(*id()); }

	std::string name() const noexcept { return git_tree_entry_name(treeEntry()); }
	git_object_t type() const noexcept { return git_tree_entry_type(treeEntry()); }
	git_filemode_t filemode() const noexcept { return git_tree_entry_filemode(treeEntry()); }

	std::optional<std::string> catFile(const Repo &repo) const noexcept;

	GitTy *treeEntry() const noexcept { return m_treeEntry.get(); }
	operator GitTy *() const noexcept { return treeEntry(); }
private:
	explicit TreeEntry(GitTy *entry, bool free = true) noexcept : m_treeEntry(entry, free) { }
	explicit TreeEntry(const GitTy *entry) noexcept :
		m_treeEntry(const_cast<GitTy *>(entry), false) { }

	Holder m_treeEntry;
};

}

#endif
