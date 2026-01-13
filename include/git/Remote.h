// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"

#include "DefaultFetchCallbacks.h"

namespace SlGit {

class Repo;

/**
 * @brief Remote is a representation of a git remote
 */
class Remote {
	using GitTy = git_remote;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	Remote() = delete;

	/// @brief Fetch \p refspecs from this Remote, invoking the \p fc callback
	bool fetchRefspecs(FetchCallbacks &fc, const std::vector<std::string> &refspecs = {},
			   int depth = 0, bool tags = true) const noexcept;
	/// @brief Fetch \p refspecs from this Remote
	bool fetchRefspecs(const std::vector<std::string> &refspecs = {}, int depth = 0,
			   bool tags = true) const noexcept {
		DefaultFetchCallbacks fc;
		return fetchRefspecs(fc, refspecs, depth, tags);
	}
	/// @brief Fetch \p branches from this Remote
	bool fetchBranches(const std::vector<std::string> &branches, int depth = 0,
			   bool tags = true) const noexcept;
	/// @brief Fetch a \p branch from this Remote
	bool fetch(const std::string &branch, int depth = 0,
		  bool tags = true) const noexcept { return fetchBranches({ branch }, depth, tags); }

	/// @brief Get the URL of this Remote
	std::string url() const noexcept { return git_remote_url(remote()); }

	/// @brief Get the stats of the progress
	const git_indexer_progress *stats() const noexcept { return git_remote_stats(remote()); }

	/// @brief Get the stored pointer to libgit2's git_remote
	GitTy *remote() const noexcept { return m_remote.get(); }
	/// @brief Alias for remote() -- implicit conversion
	operator GitTy *() const noexcept { return remote(); }
private:
	explicit Remote(GitTy *remote) noexcept : m_remote(remote) { }

	static int fetchCredentials(git_credential **out, const char *url,
				    const char *usernameFromUrl, unsigned int allowedTypes,
				    void *payload) noexcept;
	static int fetchPackProgress(int stage, uint32_t current, uint32_t total, void *payload);
	static int fetchSidebandProgress(const char *str, int len, void *payload) noexcept;
	static int fetchTransferProgress(const git_indexer_progress *stats, void *payload) noexcept;
#ifdef LIBGIT_HAS_UPDATE_REFS
	static int fetchUpdateRefs(const char *refname, const git_oid *a, const git_oid *b,
				   git_refspec *refspec, void *payload) noexcept;
#endif

	Holder m_remote;
};

}
