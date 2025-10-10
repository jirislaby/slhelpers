// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Remote.h"
#include "git/StrArray.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_remote>::operator()(git_remote *remote) const
{
	git_remote_free(remote);
}

int Remote::fetchCredentials(git_credential **out, const char *url, const char *usernameFromUrl,
			     unsigned int allowedTypes, void *payload) noexcept
{
	std::optional<std::string> username;
	if (usernameFromUrl)
		username = usernameFromUrl;
	return static_cast<FetchCallbacks *>(payload)->credentials(out, url, username,
								   allowedTypes);
}

int Remote::fetchPackProgress(int stage, uint32_t current, uint32_t total, void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->packProgress(stage, current, total);
}

int Remote::fetchSidebandProgress(const char *str, int len, void *payload) noexcept
{
	return static_cast<FetchCallbacks *>(payload)->sidebandProgress({ str, static_cast<size_t>(len) });
}

int Remote::fetchTransferProgress(const git_indexer_progress *stats, void *payload) noexcept
{
	return static_cast<FetchCallbacks *>(payload)->transferProgress(*stats);
}

#ifdef LIBGIT_HAS_UPDATE_REFS
int Remote::fetchUpdateRefs(const char *refname, const git_oid *a, const git_oid *b,
			    git_refspec *refspec, void *payload) noexcept
{
	return static_cast<FetchCallbacks *>(payload)->updateRefs(refname, *a, *b, *refspec);
}
#endif

int Remote::fetchRefspecs(FetchCallbacks &fc, const std::vector<std::string> &refspecs, int depth,
			  bool tags) const noexcept
{
	git_fetch_options opts GIT_FETCH_OPTIONS_INIT;
	opts.callbacks.payload = &fc;
	opts.callbacks.credentials = fetchCredentials;
	opts.callbacks.pack_progress = fetchPackProgress;
	opts.callbacks.sideband_progress = fetchSidebandProgress;
	opts.callbacks.transfer_progress = fetchTransferProgress;
#ifdef LIBGIT_HAS_UPDATE_REFS
	opts.callbacks.update_refs = fetchUpdateRefs;
#endif
	if (!tags)
		opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_NONE;
	opts.depth = depth;
	return git_remote_fetch(remote(), StrArray(refspecs), &opts, nullptr);
}

int Remote::fetchBranches(const std::vector<std::string> &branches, int depth,
			  bool tags) const noexcept
{
	std::string remote { git_remote_name(*this) };
	std::vector<std::string> refspecs;

	for (const auto &b : branches)
		refspecs.push_back("refs/heads/" + b + ":refs/remotes/" + remote + "/" + b);

	return fetchRefspecs(refspecs, depth, tags);
}
