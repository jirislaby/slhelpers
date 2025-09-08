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

namespace SlGit {

class Blob;
class Commit;
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
	using Holder = SlHelpers::UniqueHolder<git_repository>;
public:
	Repo() = delete;

	static std::optional<Repo> init(const std::filesystem::path &path, bool bare = false,
					const std::string &originUrl = "");
	static std::optional<Repo> clone(const std::filesystem::path &path, const std::string &url,
					 const std::string &branch = "",
					 const unsigned int &depth = 0, bool tags = true);
	static std::optional<Repo> open(const std::filesystem::path &path = ".");

	int grepBranch(const std::string &branch, const std::regex &regex) const;
	int checkout(const std::string &branch) const;
	int checkout(const Reference &reference) const;
	int checkoutTree(const Tree &tree, unsigned int strategy = GIT_CHECKOUT_SAFE) const;
	std::optional<std::string> catFile(const std::string &branch, const std::string &file) const;

	std::variant<Blob, Commit, Tag, Tree, std::monostate>
	revparseSingle(const std::string &rev) const;

	std::optional<Blob> blobCreateFromWorkDir(const std::filesystem::path &file) const;
	std::optional<Blob> blobCreateFromDisk(const std::filesystem::path &file) const;
	std::optional<Blob> blobCreateFromBuffer(const std::string &buf) const;
	std::optional<Blob> blobLookup(const git_oid &oid) const;
	std::optional<Blob> blobLookup(const TreeEntry &tentry) const;
	std::optional<Blob> blobRevparseSingle(const std::string &rev) const;

	std::optional<Commit> commitLookup(const git_oid &oid) const;
	std::optional<Commit> commitCreate(const Signature &author, const Signature &committer,
					   const std::string &msg, const Tree &tree,
					   const std::vector<const Commit *> &parents = {}) const;
	std::optional<Commit> commitCreateCheckout(const Signature &author,
						   const Signature &committer,
						   const std::string &msg, const Tree &tree,
						   unsigned int strategy = GIT_CHECKOUT_SAFE,
						   const std::vector<const Commit *> &parents = {}) const;
	std::optional<Commit> commitHead() const noexcept;
	std::optional<Commit> commitRevparseSingle(const std::string &rev) const;

	std::optional<Index> index() const;

	std::optional<Remote> remoteCreate(const std::string &name, const std::string &url) const;
	std::optional<Remote> remoteLookup(const std::string &name) const;

	std::optional<Reference> refLookup(const std::string &name) const;
	std::optional<Reference> refDWIM(const std::string &name) const;

	std::optional<Reference> refCreateDirect(const std::string &name, const git_oid &oid,
						 bool force = false) const;
	std::optional<Reference> refCreateSymbolic(const std::string &name,
						   const std::string &target,
						   bool force = false) const;

	std::optional<RevWalk> revWalkCreate() const;

	std::optional<Tag> tagCreate(const std::string &tagName, const Object &target,
				     const Signature &tagger, const std::string &message,
				     bool force = false) const;
	std::optional<Tag> tagLookup(const git_oid &oid) const;
	std::optional<Tag> tagLookup(const TreeEntry &tentry) const;
	std::optional<Tag> tagRevparseSingle(const std::string &rev) const;

	std::optional<Tree> treeLookup(const git_oid &oid) const;
	std::optional<Tree> treeLookup(const TreeEntry &tentry) const;
	std::optional<Tree> treeRevparseSingle(const std::string &rev) const;

	std::optional<TreeBuilder> treeBuilderCreate(const Tree *source = nullptr) const;

	std::filesystem::path path() const { return git_repository_path(repo()); }
	std::filesystem::path workDir() const { return git_repository_workdir(repo()); }

	git_repository *repo() const { return m_repo.get(); }
	operator git_repository *() const { return repo(); }
private:
	explicit Repo(git_repository *repo) : m_repo(repo) {}

	Holder m_repo;
};

}

#endif
