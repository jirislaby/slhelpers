// SPDX-License-Identifier: GPL-2.0-only
#ifndef DEFAULTFETCHCALLBACKS_H
#define DEFAULTFETCHCALLBACKS_H

#include "../helpers/Ratelimit.h"
#include "../helpers/SSH.h"

#include "FetchCallbacks.h"

namespace SlGit {

class DefaultFetchCallbacks : public FetchCallbacks {
public:
	DefaultFetchCallbacks() : ratelimit(std::chrono::seconds(2)), keys(SlSSH::Keys::get("")),
		tried(0), triedKey(0) { }

	virtual int credentials(git_credential **out, const std::string &url,
				const std::optional<std::string> &usernameFromUrl,
				unsigned int allowedTypes) override;
	virtual int packProgress(int stage, uint32_t current, uint32_t total) override;
	virtual int sidebandProgress(const std::string_view &str) override;
	virtual int transferProgress(const git_indexer_progress &stats) override;
	virtual int updateRefs(const std::string &refname, const git_oid &a, const git_oid &b,
			       git_refspec &) override;

private:
	std::string getUserName(const std::optional<std::string> &usernameFromUrl);

	std::string userName;
	SlHelpers::Ratelimit ratelimit;
	SlSSH::Keys::KeyPairs keys;
	unsigned int tried;
	unsigned int triedKey;
};

}

#endif
