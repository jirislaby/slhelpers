// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_TREE_H
#define GIT_TREE_H

#include <filesystem>
#include <functional>
#include <optional>
#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"

namespace SlGit {

class Blob;
class Commit;
class Repo;
class TreeEntry;

class Tree {
	using Holder = SlHelpers::UniqueHolder<git_tree>;

	friend class Commit;
	friend class Repo;
public:
	using WalkCallback = const std::function<int(const std::string &root,
		const TreeEntry &entry)>;
	Tree() = delete;

	size_t entryCount() { return git_tree_entrycount(tree()); }

	int walk(const WalkCallback &CB, const git_treewalk_mode &mode = GIT_TREEWALK_PRE);

	const git_oid *id() const { return git_tree_id(tree()); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::optional<TreeEntry> treeEntryByPath(const std::string &path) const;
	TreeEntry treeEntryByIndex(size_t idx) const;

	std::optional<std::string> catFile(const Repo &repo, const std::string &file) const;

	git_tree *tree() const { return m_tree.get(); }
	operator git_tree *() const { return tree(); }
private:
	static int walkCB(const char *root, const git_tree_entry *entry, void *payload);

	explicit Tree(git_tree *tree) : m_tree(tree) {}

	Holder m_tree;
};

class TreeBuilder {
	using Holder = SlHelpers::UniqueHolder<git_treebuilder>;

	friend class Repo;
public:
	TreeBuilder() = delete;

	int insert(const std::filesystem::path &file, const Blob &blob);
	int remove(const std::filesystem::path &file) {
		return git_treebuilder_remove(treeBuilder(), file.c_str());
	}
	int clear() { return git_treebuilder_clear(treeBuilder()); }

	std::optional<Tree> write(const Repo &repo) const;

	size_t entryCount() const { return git_treebuilder_entrycount(treeBuilder()); }

	const git_tree_entry *get(const std::filesystem::path &file) {
		return git_treebuilder_get(treeBuilder(), file.c_str());
	}

	git_treebuilder *treeBuilder() const { return m_treeBuilder.get(); }
	operator git_treebuilder *() const { return treeBuilder(); }
private:
	explicit TreeBuilder(git_treebuilder *TB) : m_treeBuilder(TB) { }

	Holder m_treeBuilder;
};

class TreeEntry {
	using Holder = SlHelpers::UniqueHolder<git_tree_entry>;

	friend class Tree;
public:
	TreeEntry() = delete;

	const git_oid *id() const { return git_tree_entry_id(treeEntry()); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::string name() const { return git_tree_entry_name(treeEntry()); }
	git_object_t type() const { return git_tree_entry_type(treeEntry()); }
	git_filemode_t filemode() const { return git_tree_entry_filemode(treeEntry()); }

	std::optional<std::string> catFile(const Repo &repo) const;

	git_tree_entry *treeEntry() const { return m_treeEntry.get(); }
	operator git_tree_entry *() const { return treeEntry(); }
private:
	explicit TreeEntry(git_tree_entry *entry, bool free = true) : m_treeEntry(entry, free) { }
	explicit TreeEntry(const git_tree_entry *entry) :
		m_treeEntry(const_cast<git_tree_entry *>(entry), false) { }

	Holder m_treeEntry;
};

}

#endif
