// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"

#include "DefaultFetchCallbacks.h"

namespace SlGit {

class Repo;

class Remote {
	using GitTy = git_remote;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	Remote() = delete;

	int fetchRefspecs(FetchCallbacks &fc, const std::vector<std::string> &refspecs = {},
			  int depth = 0, bool tags = true) const noexcept;
	int fetchRefspecs(const std::vector<std::string> &refspecs = {}, int depth = 0,
			  bool tags = true) const noexcept {
		DefaultFetchCallbacks fc;
		return fetchRefspecs(fc, refspecs, depth, tags);
	}
	int fetchBranches(const std::vector<std::string> &branches, int depth = 0,
			  bool tags = true) const noexcept;
	int fetch(const std::string &branch, int depth = 0,
		  bool tags = true) const noexcept { return fetchBranches({ branch }, depth, tags); }

	std::string url() const noexcept { return git_remote_url(remote()); }

	const git_indexer_progress *stats() const noexcept { return git_remote_stats(remote()); }

	GitTy *remote() const noexcept { return m_remote.get(); }
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
