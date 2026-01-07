// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <git2.h>

#include "../helpers/LastError.h"
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

/**
 * @brief The most important Git class
 *
 * It is the starting point to work with a git repository using this library.
 */
class Repo {
	using GitTy = git_repository;
	using Holder = SlHelpers::UniqueHolder<GitTy>;
public:
	Repo() = delete;

	/**
	 * @brief init Init an empty repository
	 * @param path Path where to create the repository
	 * @param bare Should this be bare repo (no sources, only git files)
	 * @param originUrl Set origin URL to this (or empty string)
	 * @return Repo on success, nullopt otherwise.
	 *
	 * @code{.sh} git init @endcode
	 */
	static std::optional<Repo> init(const std::filesystem::path &path, bool bare = false,
					const std::string &originUrl = "") noexcept;

	/**
	 * @brief clone Clone (and open) an existing repository
	 * @param path Path where to create the repository
	 * @param url URL where to download from
	 * @param fc Fetch callbacks, see FetchCallbacks
	 * @param branch Branch to download (or empty string)
	 * @param depth How deep to download (or zero to download whole history)
	 * @param tags Download also tags?
	 * @return Repo on success, nullopt otherwise.
	 *
	 * @code{.sh} git clone @endcode
	 */
	static std::optional<Repo> clone(const std::filesystem::path &path, const std::string &url,
					 FetchCallbacks &fc,
					 const std::string &branch = "",
					 const unsigned int &depth = 0,
					 bool tags = true) noexcept;
	/**
	 * @brief clone Clone (and open) an existing repository
	 * @param path Path where to create the repository
	 * @param url URL where to download from
	 * @param branch Branch to download (or empty string)
	 * @param depth How deep to download (or zero to download whole history)
	 * @param tags Download also tags?
	 * @return Repo on success, nullopt otherwise.
	 *
	 * @code{.sh} git clone @endcode
	 */
	static std::optional<Repo> clone(const std::filesystem::path &path, const std::string &url,
					 const std::string &branch = "",
					 const unsigned int &depth = 0,
					 bool tags = true) noexcept {
		DefaultFetchCallbacks fc;
		return clone(path, url, fc, branch, depth, tags);
	}

	/**
	 * @brief Open an existing repository
	 * @param path Path to the repository
	 * @return Repo on success, nullopt otherwise.
	 */
	static std::optional<Repo> open(const std::filesystem::path &path = ".") noexcept;

	/**
	 * @brief Update/fetch remote \p remote in repository at \p path
	 * @param path Path to an existing git repository
	 * @param remote Remote to update
	 * @return true on success.
	 *
	 * @code{.sh} git remote-update @endcode
	 */
	static bool update(const std::filesystem::path &path, const std::string &remote = "origin");

	/// @brief Checkout a \p branch
	bool checkout(const std::string &branch) const noexcept;
	/// @brief Checkout a Reference \p reference
	bool checkout(const Reference &reference) const noexcept;
	/// @brief Update index and files to match \p tree
	bool checkoutTree(const Tree &tree, unsigned int strategy = GIT_CHECKOUT_SAFE) const noexcept;
	/**
	 * @brief Cat a \p file in a \p branch
	 * @param branch Branch where to look
	 * @param file File to get content of
	 * @return File content.
	 *
	 * Kind of: @code{.sh} git cat-file @endcode
	 */
	std::optional<std::string> catFile(const std::string &branch,
					   const std::string &file) const noexcept;

	/// @brief Parse \p rev as either blob, commit, tag, or tree
	std::variant<Blob, Commit, Tag, Tree, std::monostate>
	revparseSingle(const std::string &rev) const noexcept;

	/// @brief Create a new Blob from \p file in workdir
	std::optional<Blob> blobCreateFromWorkDir(const std::filesystem::path &file) const noexcept;
	/// @brief Create a new Blob from \p file
	std::optional<Blob> blobCreateFromDisk(const std::filesystem::path &file) const noexcept;
	/// @brief Create a new Blob from string \p buf
	std::optional<Blob> blobCreateFromBuffer(const std::string &buf) const noexcept;
	/// @brief Get a Blob corresponding to \p oid
	std::optional<Blob> blobLookup(const git_oid &oid) const noexcept;
	/// @brief Get a Blob corresponding to \p tentry
	std::optional<Blob> blobLookup(const TreeEntry &tentry) const noexcept;
	/// @brief Parse \p rev as blob
	std::optional<Blob> blobRevparseSingle(const std::string &rev) const noexcept;

	/// @brief Get a Commit corresponding to \p oid
	std::optional<Commit> commitLookup(const git_oid &oid) const noexcept;
	/// @brief Create a new Commit
	std::optional<Commit> commitCreate(const Signature &author, const Signature &committer,
					   const std::string &msg, const Tree &tree,
					   const std::vector<const Commit *> &parents = {}) const noexcept;
	/// @brief Create a new Commit and move to it
	std::optional<Commit> commitCreateCheckout(const Signature &author,
						   const Signature &committer,
						   const std::string &msg, const Tree &tree,
						   unsigned int strategy = GIT_CHECKOUT_SAFE,
						   const std::vector<const Commit *> &parents = {}) const noexcept;
	/// @brief Get a Commit corresponding to HEAD
	std::optional<Commit> commitHead() const noexcept;
	/// @brief Parse \p rev as commit
	std::optional<Commit> commitRevparseSingle(const std::string &rev) const noexcept;

	/// @brief Create a new Diff of \p commit1 and \p commit2
	/// @code{.sh} git diff commit1..commit2 @endcode
	std::optional<Diff> diff(const Commit &commit1, const Commit &commit2,
				 const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of \p tree1 and \p tree2
	/// @code{.sh} git diff tree1..tree2 @endcode
	std::optional<Diff> diff(const Tree &tree1, const Tree &tree2,
				 const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of a \p commit and \p index
	/// @code{.sh} git diff --cached commit @endcode
	std::optional<Diff> diffCached(const Commit &commit, const Index &index,
				       const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of a \p tree and \p index
	/// @code{.sh} git diff --cached tree @endcode
	std::optional<Diff> diffCached(const Tree &tree, const Index &index,
				       const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of a \p commit and repo's index
	std::optional<Diff> diffCached(const Commit &commit,
				       const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of a \p tree and repo's index
	std::optional<Diff> diffCached(const Tree &tree,
				       const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of an \p index and workdir
	/// @code{.sh} git diff @endcode
	std::optional<Diff> diffWorkdir(const Index &index,
					const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of a \p commit and workdir
	std::optional<Diff> diffWorkdir(const Commit &commit,
					const git_diff_options *opts = nullptr) const noexcept;
	/// @brief Create a new Diff of a \p tree and workdir
	std::optional<Diff> diffWorkdir(const Tree &tree,
					const git_diff_options *opts = nullptr) const noexcept;

	/// @brief Get repository's index
	std::optional<Index> index() const noexcept;

	/// @brief Create a new Remote called \p name located at \p url
	std::optional<Remote> remoteCreate(const std::string &name,
					   const std::string &url) const noexcept;
	/// @brief Get a Remote called \p name
	std::optional<Remote> remoteLookup(const std::string &name) const noexcept;

	/// @brief Get a Reference called exactly \p name (like \c refs/heads/master)
	std::optional<Reference> refLookup(const std::string &name) const noexcept;
	/// @brief Get a Reference called \p name (like \c master)
	std::optional<Reference> refDWIM(const std::string &name) const noexcept;

	/// @brief Create a new direct Reference to \p oid called \p name
	std::optional<Reference> refCreateDirect(const std::string &name, const git_oid &oid,
						 bool force = false) const noexcept;
	/// @brief Create a new symbolic Reference to \p target called \p name
	std::optional<Reference> refCreateSymbolic(const std::string &name,
						   const std::string &target,
						   bool force = false) const noexcept;

	/// @brief Create a new RevWalk
	std::optional<RevWalk> revWalkCreate() const noexcept;

	/// @brief Create a new Tag called \p tagName, pointing at \p target
	std::optional<Tag> tagCreate(const std::string &tagName, const Object &target,
				     const Signature &tagger, const std::string &message,
				     bool force = false) const noexcept;
	/// @brief Get a Tag corresponding to \p oid
	std::optional<Tag> tagLookup(const git_oid &oid) const noexcept;
	/// @brief Get a Tag corresponding to \p tentry
	std::optional<Tag> tagLookup(const TreeEntry &tentry) const noexcept;
	/// @brief Parse \p rev as tag
	std::optional<Tag> tagRevparseSingle(const std::string &rev) const noexcept;

	/// @brief Get a Tree corresponding to \p oid
	std::optional<Tree> treeLookup(const git_oid &oid) const noexcept;
	/// @brief Get a Tree corresponding to \p tentry
	std::optional<Tree> treeLookup(const TreeEntry &tentry) const noexcept;
	/// @brief Parse \p rev as tree
	std::optional<Tree> treeRevparseSingle(const std::string &rev) const noexcept;

	/// @brief Create a new TreeBuilder
	std::optional<TreeBuilder> treeBuilderCreate(const Tree *source = nullptr) const noexcept;

	/// @brief Get the path to .git
	std::filesystem::path path() const noexcept { return git_repository_path(repo()); }
	/// @brief Get the path to sources
	std::filesystem::path workDir() const noexcept { return git_repository_workdir(repo()); }

	/// @brief Return the last error string if some (from git_last_error())
	static auto &lastError() noexcept { return m_lastError.lastError(); }
	/// @brief Return the last error class (from git_last_error())
	static auto lastClass() noexcept { return m_lastError.get<0>(); }
	/// @brief Return the last error number (returned from git_* functions)
	static auto lastErrno() noexcept { return m_lastError.get<1>(); }

	/// @brief Get the underlying libgit2 pointer
	GitTy *repo() const noexcept { return m_repo.get(); }
	/// @brief Get the underlying libgit2 pointer
	operator GitTy *() const noexcept { return repo(); }
private:
	explicit Repo(GitTy *repo) noexcept : m_repo(repo) {}

	friend class Diff;
	friend class Index;
	friend class PathSpec;
	friend class Remote;
	friend class RevWalk;
	friend class Signature;
	friend class Tag;
	friend class Tree;
	friend class TreeBuilder;

	static std::pair<std::string, int> lastGitError() noexcept;

	static int setLastError(int ret) {
		if (ret) {
			auto le = lastGitError();
			m_lastError.reset().setError(std::move(le.first));
			m_lastError.set<0>(le.second);
			m_lastError.set<1>(ret);
		}

		return ret;
	}

	template<class Class, typename FunTy, typename... Args>
	static std::optional<Class> MakeGit(const FunTy &fun, Args&&... args)
	{
		typename Class::GitTy *gitEntry;
		if (setLastError(fun(&gitEntry, std::forward<Args>(args)...)))
			return std::nullopt;
		return Class(gitEntry);
	}

	template<class Class, typename FunTy, typename... Args>
	static std::optional<Class> MakeGitRepo(const Repo &repo, const FunTy &fun, Args&&... args)
	{
		typename Class::GitTy *gitEntry;
		if (setLastError(fun(&gitEntry, std::forward<Args>(args)...)))
			return std::nullopt;
		return Class(repo, gitEntry);
	}

	static void checkoutProgress(const char *path, size_t completed_steps, size_t total_steps,
				     void *payload);

	Holder m_repo;
	static thread_local SlHelpers::LastErrorStr<int, int> m_lastError;
};

}
