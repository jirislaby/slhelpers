// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string_view>

#include <git2.h>

namespace SlGit {

class FetchCallbacks {
public:
	FetchCallbacks() {}

	virtual void checkoutProgress(std::string_view, size_t, size_t) {}

	virtual int credentials(git_credential **, std::string_view,
				std::optional<std::string_view>, unsigned int) {
		return GIT_PASSTHROUGH;
	}
	virtual int packProgress(int, uint32_t, uint32_t) {
		return GIT_PASSTHROUGH;
	}
	virtual int sidebandProgress(std::string_view) {
		return GIT_PASSTHROUGH;
	}
	virtual int transferProgress(const git_indexer_progress &) {
		return GIT_PASSTHROUGH;
	}
	virtual int updateRefs(std::string_view, const git_oid &, const git_oid &,
			       git_refspec &) {
		return GIT_PASSTHROUGH;
	}
};

}
