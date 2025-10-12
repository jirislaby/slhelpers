// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_REPO_H
#define GIT_REPO_H

#include <filesystem>
#include <optional>
#include <regex>
#include <string>
#include <variant>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"

#include "DefaultFetchCallbacks.h"

namespace SlGit {

class Blob;
class Commit;
class Diff;
class Index;
class Object;
class Reference;
class Remote;
class RevWalk;
class Signature;
class Tag;
class Tree;
class TreeBuilder;
class TreeEntry;

class Repo {
	using GitTy = git_repository;
	using Holder = SlHelpers::UniqueHolder<GitTy>;
public:
	Repo() = delete;

	static std::optional<Repo> init(const std::filesystem::path &path, bool bare = false,
					const std::string &originUrl = "") noexcept;
	static std::optional<Repo> clone(const std::filesystem::path &path, const std::string &url,
					 FetchCallbacks &fc,
					 const std::string &branch = "",
					 const unsigned int &depth = 0,
					 bool tags = true) noexcept;
	static std::optional<Repo> clone(const std::filesystem::path &path, const std::string &url,
					 const std::string &branch = "",
					 const unsigned int &depth = 0,
					 bool tags = true) noexcept {
		DefaultFetchCallbacks fc;
		return clone(path, url, fc, branch, depth, tags);
	}
	static std::optional<Repo> open(const std::filesystem::path &path = ".") noexcept;

	int grepBranch(const std::string &branch, const std::regex &regex) const noexcept;
	int checkout(const std::string &branch) const noexcept;
	int checkout(const Reference &reference) const noexcept;
	int checkoutTree(const Tree &tree, unsigned int strategy = GIT_CHECKOUT_SAFE) const noexcept;
	std::optional<std::string> catFile(const std::string &branch,
					   const std::string &file) const noexcept;

	std::variant<Blob, Commit, Tag, Tree, std::monostate>
	revparseSingle(const std::string &rev) const noexcept;

	std::optional<Blob> blobCreateFromWorkDir(const std::filesystem::path &file) const noexcept;
	std::optional<Blob> blobCreateFromDisk(const std::filesystem::path &file) const noexcept;
	std::optional<Blob> blobCreateFromBuffer(const std::string &buf) const noexcept;
	std::optional<Blob> blobLookup(const git_oid &oid) const noexcept;
	std::optional<Blob> blobLookup(const TreeEntry &tentry) const noexcept;
	std::optional<Blob> blobRevparseSingle(const std::string &rev) const noexcept;

	std::optional<Commit> commitLookup(const git_oid &oid) const noexcept;
	std::optional<Commit> commitCreate(const Signature &author, const Signature &committer,
					   const std::string &msg, const Tree &tree,
					   const std::vector<const Commit *> &parents = {}) const noexcept;
	std::optional<Commit> commitCreateCheckout(const Signature &author,
						   const Signature &committer,
						   const std::string &msg, const Tree &tree,
						   unsigned int strategy = GIT_CHECKOUT_SAFE,
						   const std::vector<const Commit *> &parents = {}) const noexcept;
	std::optional<Commit> commitHead() const noexcept;
	std::optional<Commit> commitRevparseSingle(const std::string &rev) const noexcept;

	// two commits/trees: git diff commit1..commit2
	std::optional<Diff> diff(const Commit &commit1, const Commit &commit2,
				 const git_diff_options *opts = nullptr) const noexcept;
	std::optional<Diff> diff(const Tree &tree1, const Tree &tree2,
				 const git_diff_options *opts = nullptr) const noexcept;
	// commit/tree to index: git diff --cached commit
	std::optional<Diff> diffCached(const Commit &commit, const Index &index,
				       const git_diff_options *opts = nullptr) const noexcept;
	std::optional<Diff> diffCached(const Tree &tree, const Index &index,
				       const git_diff_options *opts = nullptr) const noexcept;
	// commit/tree to repo's index
	std::optional<Diff> diffCached(const Commit &commit,
				       const git_diff_options *opts = nullptr) const noexcept;
	std::optional<Diff> diffCached(const Tree &tree,
				       const git_diff_options *opts = nullptr) const noexcept;
	// index to workdir: git diff
	std::optional<Diff> diffWorkdir(const Index &index,
					const git_diff_options *opts = nullptr) const noexcept;
	std::optional<Diff> diffWorkdir(const Commit &commit,
					const git_diff_options *opts = nullptr) const noexcept;
	std::optional<Diff> diffWorkdir(const Tree &tree,
					const git_diff_options *opts = nullptr) const noexcept;

	std::optional<Index> index() const noexcept;

	std::optional<Remote> remoteCreate(const std::string &name,
					   const std::string &url) const noexcept;
	std::optional<Remote> remoteLookup(const std::string &name) const noexcept;

	std::optional<Reference> refLookup(const std::string &name) const noexcept;
	std::optional<Reference> refDWIM(const std::string &name) const noexcept;

	std::optional<Reference> refCreateDirect(const std::string &name, const git_oid &oid,
						 bool force = false) const noexcept;
	std::optional<Reference> refCreateSymbolic(const std::string &name,
						   const std::string &target,
						   bool force = false) const noexcept;

	std::optional<RevWalk> revWalkCreate() const noexcept;

	std::optional<Tag> tagCreate(const std::string &tagName, const Object &target,
				     const Signature &tagger, const std::string &message,
				     bool force = false) const noexcept;
	std::optional<Tag> tagLookup(const git_oid &oid) const noexcept;
	std::optional<Tag> tagLookup(const TreeEntry &tentry) const noexcept;
	std::optional<Tag> tagRevparseSingle(const std::string &rev) const noexcept;

	std::optional<Tree> treeLookup(const git_oid &oid) const noexcept;
	std::optional<Tree> treeLookup(const TreeEntry &tentry) const noexcept;
	std::optional<Tree> treeRevparseSingle(const std::string &rev) const noexcept;

	std::optional<TreeBuilder> treeBuilderCreate(const Tree *source = nullptr) const noexcept;

	std::filesystem::path path() const noexcept { return git_repository_path(repo()); }
	std::filesystem::path workDir() const noexcept { return git_repository_workdir(repo()); }

	GitTy *repo() const noexcept { return m_repo.get(); }
	operator GitTy *() const noexcept { return repo(); }
private:
	explicit Repo(GitTy *repo) noexcept : m_repo(repo) {}

	Holder m_repo;

	template<class Class, typename FunTy, typename... Args>
	static std::optional<Class> MakeGit(const FunTy &fun, Args&&... args)
	{
		typename Class::GitTy *gitEntry;
		if (fun(&gitEntry, std::forward<Args>(args)...))
			return std::nullopt;
		return Class(gitEntry);
	}

	static void checkoutProgress(const char *path, size_t completed_steps, size_t total_steps,
				     void *payload);
};

}

#endif
