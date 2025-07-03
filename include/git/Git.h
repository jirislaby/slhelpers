#ifndef GIT_H
#define GIT_H

#include <filesystem>
#include <optional>
#include <regex>
#include <vector>

#include <git2.h>

namespace SlGit {

class Helpers {
public:
	static std::string oidToStr(const git_oid &id) {
		char buf[GIT_OID_MAX_HEXSIZE + 1];
		git_oid_tostr(buf, sizeof(buf), &id);
		return buf;
	}
};

class FetchCallbacks {
public:
	FetchCallbacks() {}

	virtual int credentials(git_credential **, const std::string &,
				const std::optional<std::string> &, unsigned int) {
		return GIT_PASSTHROUGH;
	}
	virtual int packProgress(int, uint32_t, uint32_t) {
		return GIT_PASSTHROUGH;
	}
	virtual int sidebandProgress(const std::string_view &) {
		return GIT_PASSTHROUGH;
	}
	virtual int transferProgress(const git_indexer_progress &) {
		return GIT_PASSTHROUGH;
	}
	virtual int updateRefs(const std::string &, const git_oid &, const git_oid &,
			       git_refspec &) {
		return GIT_PASSTHROUGH;
	}
};

class Repo
{
public:
	Repo();
	~Repo();

	int init(const std::filesystem::path &path, bool bare = false,
		 const std::string &origin = "");
	int clone(const std::filesystem::path &path, const std::string &url,
		  const std::string &branch = "",
		  const unsigned int &depth = 0, bool tags = true);
	int open(const std::filesystem::path &path = ".");
	int grepBranch(const std::string &branch, const std::regex &regex);
	int checkout(const std::string &branch);
	std::optional<std::string> catFile(const std::string &branch, const std::string &file);

	operator git_repository *() const { return repo; }
private:
	git_repository *repo;
};

class Remote
{
public:
	Remote() : remote(nullptr) {}
	~Remote() { git_remote_free(remote); }

	int lookup(const Repo &repo, const std::string &name) {
		return git_remote_lookup(&remote, repo, name.c_str());
	}
	int fetchRefspecs(const std::vector<std::string> &refspecs = {}, int depth = 0,
			  bool tags = true);
	int fetchBranches(const std::vector<std::string> &branches, int depth = 0,
			  bool tags = true);
	int fetch(const std::string &branch, int depth = 0,
		  bool tags = true) { return fetchBranches({ branch }, depth, tags); }
	operator git_remote *() const { return remote; }
private:
	git_remote *remote;
};

class Index {
public:
	Index() : index(nullptr) { }
	Index(git_index *index) : index(index) { }
	~Index() { git_index_free(index); }

	int repoIndex(const Repo &repo) { return git_repository_index(&index, repo); }

	operator git_index *() const { return index; }
private:
	git_index *index;
};

class Commit {
public:
	Commit() : commit(nullptr) { }
	~Commit() { git_commit_free(commit); }

	int lookup(const Repo &repo, const git_oid &oid) {
		return git_commit_lookup(&commit, repo, &oid);
	}

	int revparseSingle(const Repo &repo, const std::string &rev) {
		return git_revparse_single((git_object **)&commit, repo, rev.c_str());
	}

	const git_oid *id() const { return git_commit_id(commit); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	operator git_commit *() const { return commit; }
private:
	git_commit *commit;
};

class TreeEntry;

class Tree {
public:
	using WalkCallback = const std::function<int(const std::string &root,
		const TreeEntry &entry)>;
	Tree() : tree(nullptr) { }
	~Tree() { git_tree_free(tree); }

	int ofCommit(const Commit &commit) {
		return git_commit_tree(&tree, commit);
	}
	int lookup(const Repo &repo, const git_oid &id) {
		return git_tree_lookup(&tree, repo, &id);
	}
	int lookup(const Repo &repo, const TreeEntry &entry);

	size_t entryCount() { return git_tree_entrycount(tree); }

	int walk(const WalkCallback &CB, const git_treewalk_mode &mode = GIT_TREEWALK_PRE);

	operator git_tree *() const { return tree; }
private:
	git_tree *tree;
};

class TreeEntry {
public:
	TreeEntry() : treeEntry(nullptr), free(true) { }
	TreeEntry(const git_tree_entry *entry) : treeEntry(const_cast<git_tree_entry *>(entry)),
		free(false) { }
	~TreeEntry() {
		if (free)
			git_tree_entry_free(treeEntry);
	}

	int byPath(const Tree &tree, const std::string &path) {
		return git_tree_entry_bypath(&treeEntry, tree, path.c_str());
	}
	void byIndex(const Tree &tree, size_t idx) {
		treeEntry = const_cast<git_tree_entry *>(git_tree_entry_byindex(tree, idx));
	}

	const git_oid *id() const { return git_tree_entry_id(treeEntry); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::string name() const { return git_tree_entry_name(treeEntry); }
	git_object_t type() const { return git_tree_entry_type(treeEntry); }
	git_filemode_t filemode() const { return git_tree_entry_filemode(treeEntry); }

	operator git_tree_entry *() const { return treeEntry; }
private:
	git_tree_entry *treeEntry;
	bool free;
};

inline int Tree::lookup(const Repo &repo, const TreeEntry &entry) {
	return lookup(repo, *entry.id());
}

class Blob {
public:
	Blob() : blob(nullptr) { }
	~Blob() { git_blob_free(blob); }

	int lookup(const Repo &repo, const TreeEntry &tentry) {
		return git_blob_lookup(&blob, repo, tentry.id());
	}

	std::string content() { return std::string(static_cast<const char *>(rawcontent()),
						   rawsize()); }

	operator git_blob *() const { return blob; }
private:
	git_object_size_t rawsize() const { return git_blob_rawsize(blob); }
	const void *rawcontent() const { return git_blob_rawcontent(blob); }

	git_blob *blob;
};

class Reference {
public:
	Reference() : ref(nullptr) { }
	~Reference() { git_reference_free(ref); }

	int lookup(const Repo &repo, const std::string &name) {
		return git_reference_lookup(&ref, repo, name.c_str());
	}
	int dwim(const Repo &repo, const std::string &name) {
		return git_reference_dwim(&ref, repo, name.c_str());
	}

	int createDirect(const Repo &repo, const std::string &name, const git_oid &oid,
			 bool force = false) {
		return git_reference_create(&ref, repo, name.c_str(), &oid, force, nullptr);
	}
	int createSymbolic(const Repo &repo, const std::string &name, const std::string &target,
			   bool force = false) {
		return git_reference_symbolic_create(&ref, repo, name.c_str(), target.c_str(),
						     force, nullptr);
	}

	const git_oid *target() const { return git_reference_target(ref); }

	operator git_reference *() const { return ref; }
private:
	git_reference *ref;
};


}

#endif // GIT_H
