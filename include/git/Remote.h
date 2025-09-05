// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_REMOTE_H
#define GIT_REMOTE_H

#include <string>
#include <vector>

#include <git2.h>

#include "../helpers/Unique.h"

namespace SlGit {

class Repo;

class Remote {
	using Holder = SlHelpers::UniqueHolder<git_remote>;

	friend class Repo;
public:
	Remote() = delete;

	int fetchRefspecs(const std::vector<std::string> &refspecs = {}, int depth = 0,
			  bool tags = true);
	int fetchBranches(const std::vector<std::string> &branches, int depth = 0,
			  bool tags = true);
	int fetch(const std::string &branch, int depth = 0,
		  bool tags = true) { return fetchBranches({ branch }, depth, tags); }

	std::string url() const { return git_remote_url(remote()); }

	git_remote *remote() const { return m_remote.get(); }
	operator git_remote *() const { return remote(); }
private:
	explicit Remote(git_remote *remote) : m_remote(remote) { }

	static int fetchCredentials(git_credential **out, const char *url,
				    const char *usernameFromUrl, unsigned int allowedTypes,
				    void *payload);
	static int fetchPackProgress(int stage, uint32_t current, uint32_t total, void *payload);
	static int fetchSidebandProgress(const char *str, int len, void *payload);
	static int fetchTransferProgress(const git_indexer_progress *stats, void *payload);
#ifdef LIBGIT_HAS_UPDATE_REFS
	static int fetchUpdateRefs(const char *refname, const git_oid *a, const git_oid *b,
				   git_refspec *refspec, void *payload);
#endif

	Holder m_remote;
};

}

#endif
