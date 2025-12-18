// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "../helpers/Ratelimit.h"
#include "../helpers/SSH.h"

#include "FetchCallbacks.h"

namespace SlGit {

class DefaultFetchCallbacks : public FetchCallbacks {
public:
	DefaultFetchCallbacks() : ratelimit(std::chrono::seconds(2)), keys(SlSSH::Keys::get("")),
		tried(0), triedKey(0) { }

	virtual void checkoutProgress(std::string_view, size_t, size_t) override;

	virtual int credentials(git_credential **out, std::string_view url,
				std::optional<std::string_view> usernameFromUrl,
				unsigned int allowedTypes) override;
	virtual int packProgress(int stage, uint32_t current, uint32_t total) override;
	virtual int sidebandProgress(std::string_view str) override;
	virtual int transferProgress(const git_indexer_progress &stats) override;
	virtual int updateRefs(std::string_view refname, const git_oid &a, const git_oid &b,
			       git_refspec &) override;

private:
	std::string getUserName(std::optional<std::string_view> usernameFromUrl);

	std::string userName;
	SlHelpers::Ratelimit ratelimit;
	SlSSH::Keys::KeyPairs keys;
	unsigned int tried;
	unsigned int triedKey;
};

}
