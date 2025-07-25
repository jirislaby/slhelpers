// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_H
#define GIT_H

#include <filesystem>
#include <optional>
#include <regex>
#include <vector>

#include <git2.h>

namespace SlGit {

class Tree;
class TreeEntry;
class Blob;

class Helpers {
public:
	static std::string oidToStr(const git_oid &id) {
		char buf[GIT_OID_MAX_HEXSIZE + 1];
		git_oid_tostr(buf, sizeof(buf), &id);
		return buf;
	}

};

class StrArray {
public:
	StrArray(const std::vector<std::string> &vec) {
		for (const auto &entry : vec)
			strings.push_back(entry.c_str());
		m_array.strings = const_cast<char **>(strings.data());
		m_array.count = strings.size();
	}

	const git_strarray *array() const { return &m_array; }
	operator const git_strarray *() const { return &m_array; }
private:
	std::vector<const char *> strings;
	git_strarray m_array;
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

class Signature {
public:
	Signature() : m_signature(nullptr) { }
	~Signature() { git_signature_free(m_signature); }

	int now(const std::string &name, const std::string &email) {
		return git_signature_now(&m_signature, name.c_str(), email.c_str());
	}

	git_signature *signature() const { return m_signature; }
	operator git_signature *() const { return m_signature; }
private:
	git_signature *m_signature;
};

class Repo {
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
	int checkoutTree(const Tree &tree, unsigned int strategy = GIT_CHECKOUT_SAFE);
	std::optional<std::string> catFile(const std::string &branch, const std::string &file) const;

	git_repository *repo() const { return m_repo; }
	operator git_repository *() const { return m_repo; }
private:
	git_repository *m_repo;
};

class Remote {
public:
	Remote() : m_remote(nullptr) {}
	~Remote() { git_remote_free(m_remote); }

	int lookup(const Repo &repo, const std::string &name) {
		return git_remote_lookup(&m_remote, repo, name.c_str());
	}
	int fetchRefspecs(const std::vector<std::string> &refspecs = {}, int depth = 0,
			  bool tags = true);
	int fetchBranches(const std::vector<std::string> &branches, int depth = 0,
			  bool tags = true);
	int fetch(const std::string &branch, int depth = 0,
		  bool tags = true) { return fetchBranches({ branch }, depth, tags); }

	git_remote *remote() const { return m_remote; }
	operator git_remote *() const { return m_remote; }
private:
	git_remote *m_remote;
};

class Commit {
public:
	Commit() : m_commit(nullptr) { }
	~Commit() { git_commit_free(m_commit); }

	int lookup(const Repo &repo, const git_oid &oid) {
		return git_commit_lookup(&m_commit, repo, &oid);
	}
	int parent(const Commit &ofCommit, unsigned int nth) {
		return git_commit_parent(&m_commit, ofCommit, nth);
	}
	int ancestor(const Commit &ofCommit, unsigned int nth) {
		return git_commit_nth_gen_ancestor(&m_commit, ofCommit, nth);
	}

	int revparseSingle(const Repo &repo, const std::string &rev) {
		return git_revparse_single((git_object **)&m_commit, repo, rev.c_str());
	}

	int create(const Repo &repo, const Signature &author, const Signature &committer,
		   const std::string &msg, const Tree &tree,
		   const std::vector<const Commit *> &parents = {});
	int createCheckout(Repo &repo, const Signature &author, const Signature &committer,
			   const std::string &msg, const Tree &tree,
			   unsigned int strategy = GIT_CHECKOUT_SAFE,
			   const std::vector<const Commit *> &parents = {}) {
		auto ret = create(repo, author, committer, msg, tree, parents);
		if (ret)
			return ret;
		return repo.checkoutTree(tree, strategy);
	}

	const git_oid *id() const { return git_commit_id(m_commit); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }
	const git_oid *treeId() const { return git_commit_tree_id(m_commit); }
	std::string treeIdStr() const { return Helpers::oidToStr(*treeId()); }
	std::string messageEncoding() const { return git_commit_message_encoding(m_commit); }
	std::string message() const { return git_commit_message(m_commit); }
	std::string summary() const { return git_commit_summary(m_commit); }
	git_time_t time() const { return git_commit_time(m_commit); }
	int timeOffset() const { return git_commit_time_offset(m_commit); }
	const git_signature *committer() const { return git_commit_committer(m_commit); }
	const git_signature *author() const { return git_commit_author(m_commit); }
	std::string rawHeader() const { return git_commit_raw_header(m_commit); }

	unsigned int parentCount() const { return git_commit_parentcount(m_commit); }
	const git_oid *parentId(unsigned int nth) const { return git_commit_parent_id(m_commit, nth); }

	std::optional<std::string> catFile(const Repo &repo, const std::string &file) const;

	git_commit *commit() const { return m_commit; }
	operator git_commit *() const { return m_commit; }
private:
	git_commit *m_commit;
};

class Tree {
public:
	using WalkCallback = const std::function<int(const std::string &root,
		const TreeEntry &entry)>;
	Tree() : m_tree(nullptr) { }
	~Tree() { git_tree_free(m_tree); }

	int ofCommit(const Commit &commit) {
		return git_commit_tree(&m_tree, commit);
	}
	int lookup(const Repo &repo, const git_oid &id) {
		return git_tree_lookup(&m_tree, repo, &id);
	}
	int lookup(const Repo &repo, const TreeEntry &entry);

	size_t entryCount() { return git_tree_entrycount(m_tree); }

	int walk(const WalkCallback &CB, const git_treewalk_mode &mode = GIT_TREEWALK_PRE);

	const git_oid *id() const { return git_tree_id(m_tree); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::optional<std::string> catFile(const Repo &repo, const std::string &file) const;

	git_tree *tree() const { return m_tree; }
	operator git_tree *() const { return m_tree; }
private:
	static int walkCB(const char *root, const git_tree_entry *entry, void *payload);
	git_tree *m_tree;
};

class TreeEntry {
public:
	TreeEntry() : m_treeEntry(nullptr), free(true) { }
	TreeEntry(const git_tree_entry *entry) : m_treeEntry(const_cast<git_tree_entry *>(entry)),
		free(false) { }
	~TreeEntry() {
		if (free)
			git_tree_entry_free(m_treeEntry);
	}

	int byPath(const Tree &tree, const std::string &path) {
		return git_tree_entry_bypath(&m_treeEntry, tree, path.c_str());
	}
	void byIndex(const Tree &tree, size_t idx) {
		m_treeEntry = const_cast<git_tree_entry *>(git_tree_entry_byindex(tree, idx));
	}

	const git_oid *id() const { return git_tree_entry_id(m_treeEntry); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::string name() const { return git_tree_entry_name(m_treeEntry); }
	git_object_t type() const { return git_tree_entry_type(m_treeEntry); }
	git_filemode_t filemode() const { return git_tree_entry_filemode(m_treeEntry); }

	std::optional<std::string> catFile(const Repo &repo) const;

	git_tree_entry *treeEntry() const { return m_treeEntry; }
	operator git_tree_entry *() const { return m_treeEntry; }
private:
	git_tree_entry *m_treeEntry;
	bool free;
};

inline int Tree::lookup(const Repo &repo, const TreeEntry &entry) {
	return lookup(repo, *entry.id());
}

class Index {
public:
	using MatchCB = std::function<int(const std::filesystem::path &, const std::string &)>;

	Index() : m_index(nullptr) { }
	Index(git_index *m_index) : m_index(m_index) { }
	~Index() { git_index_free(m_index); }

	int repoIndex(const Repo &repo) { return git_repository_index(&m_index, repo); }
	int byPath(const std::filesystem::path &path) {
		return git_index_open(&m_index, path.c_str());
	}
	int create() { return git_index_new(&m_index); }

	int read(bool force = true) { return git_index_read(m_index, force); }
	int write() { return git_index_write(m_index); }

	int readTree(const Tree &tree) { return git_index_read_tree(m_index, tree); }
	int writeTree(const Repo &repo, Tree &tree) {
		git_oid oid;
		auto ret = git_index_write_tree(&oid, m_index);
		if (ret)
			return ret;
		return tree.lookup(repo, oid);
	}

	const git_index_entry *entryByIndex(size_t idx) const {
		return git_index_get_byindex(m_index, idx);
	}
	const git_index_entry *entryByPath(const std::filesystem::path &path,
					   git_index_stage_t stage = GIT_INDEX_STAGE_NORMAL) const {
		return git_index_get_bypath(m_index, path.c_str(), stage);
	}

	int addByPath(const std::filesystem::path &path) {
		return git_index_add_bypath(m_index, path.c_str());
	}
	int removeByPath(const std::filesystem::path &path) {
		return git_index_remove_bypath(m_index, path.c_str());
	}
	int addAll(const std::vector<std::string> &paths, unsigned int flags, const MatchCB &cb);
	int removeAll(const std::vector<std::string> &paths, const MatchCB &cb);
	int updateAll(const std::vector<std::string> &paths, const MatchCB &cb);

	bool hasConflicts() const { return git_index_has_conflicts(m_index); }

	git_index *index() const { return m_index; }
	operator git_index *() const { return m_index; }
private:
	static int matchCB(const char *path, const char *matched_pathspec, void *payload);
	git_index *m_index;
};

class TreeBuilder {
public:
	TreeBuilder() : m_treeBuilder(nullptr) { }
	~TreeBuilder() { git_treebuilder_free(m_treeBuilder); }

	int create(const Repo &repo, const Tree *source = nullptr) {
		return git_treebuilder_new(&m_treeBuilder, repo, source ? source->tree() : nullptr);
	}

	int insert(const std::filesystem::path &file, const Blob &blob);
	int remove(const std::filesystem::path &file) {
		return git_treebuilder_remove(m_treeBuilder, file.c_str());
	}
	int clear() { return git_treebuilder_clear(m_treeBuilder); }

	int write(const Repo &repo, Tree &tree) {
		git_oid oid;
		auto ret = git_treebuilder_write(&oid, m_treeBuilder);
		if (ret)
			return ret;
		return tree.lookup(repo, oid);
	}

	size_t entryCount() const { return git_treebuilder_entrycount(m_treeBuilder); }

	const git_tree_entry *get(const std::filesystem::path &file) {
		return git_treebuilder_get(m_treeBuilder, file.c_str());
	}

	git_treebuilder *treeBuilder() const { return m_treeBuilder; }
	operator git_treebuilder *() const { return m_treeBuilder; }
private:
	git_treebuilder *m_treeBuilder;
};

class Blob {
public:
	Blob() : m_blob(nullptr) { }
	~Blob() { git_blob_free(m_blob); }

	int lookup(const Repo &repo, const git_oid &oid) {
		return git_blob_lookup(&m_blob, repo, &oid);
	}
	int lookup(const Repo &repo, const TreeEntry &tentry) {
		return git_blob_lookup(&m_blob, repo, tentry.id());
	}
	int createFromWorkDir(const Repo &repo, const std::filesystem::path &file) {
		git_oid oid;
		auto ret = git_blob_create_from_workdir(&oid, repo, file.c_str());
		if (ret)
			return ret;
		return lookup(repo, oid);
	}
	int createFromDisk(const Repo &repo, const std::filesystem::path &file) {
		git_oid oid;
		auto ret = git_blob_create_from_disk(&oid, repo, file.c_str());
		if (ret)
			return ret;
		return lookup(repo, oid);
	}
	int createFromBuffer(const Repo &repo, const std::string &buf) {
		git_oid oid;
		auto ret = git_blob_create_from_buffer(&oid, repo, buf.c_str(), buf.length());
		if (ret)
			return ret;
		return lookup(repo, oid);
	}

	const git_oid *id() const { return git_blob_id(m_blob); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	std::string content() const {
		return std::string(static_cast<const char *>(rawcontent()), rawsize());
	}
	std::string_view contentView() const {
		return std::string_view(static_cast<const char *>(rawcontent()), rawsize());
	}

	git_blob *blob() const { return m_blob; }
	operator git_blob *() const { return m_blob; }
private:
	git_object_size_t rawsize() const { return git_blob_rawsize(m_blob); }
	const void *rawcontent() const { return git_blob_rawcontent(m_blob); }

	git_blob *m_blob;
};

class Reference {
public:
	Reference() : m_ref(nullptr) { }
	~Reference() { git_reference_free(m_ref); }

	int lookup(const Repo &repo, const std::string &name) {
		return git_reference_lookup(&m_ref, repo, name.c_str());
	}
	int dwim(const Repo &repo, const std::string &name) {
		return git_reference_dwim(&m_ref, repo, name.c_str());
	}

	int createDirect(const Repo &repo, const std::string &name, const git_oid &oid,
			 bool force = false) {
		return git_reference_create(&m_ref, repo, name.c_str(), &oid, force, nullptr);
	}
	int createSymbolic(const Repo &repo, const std::string &name, const std::string &target,
			   bool force = false) {
		return git_reference_symbolic_create(&m_ref, repo, name.c_str(), target.c_str(),
						     force, nullptr);
	}

	const git_oid *target() const { return git_reference_target(m_ref); }

	git_reference *ref() const { return m_ref; }
	operator git_reference *() const { return m_ref; }
private:
	git_reference *m_ref;
};

}

#endif // GIT_H
