// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_FETCHCALLBACKS_H
#define SLGIT_FETCHCALLBACKS_H

#include <optional>
#include <string>

#include <git2.h>

namespace SlGit {

class FetchCallbacks {
public:
	FetchCallbacks() {}

	virtual void checkoutProgress(const std::string &, size_t, size_t) {}

	virtual int credentials(git_credential **, const std::string &,
				const std::optional<std::string> &, unsigned int) {
		return GIT_PASSTHROUGH;
	}
	virtual int packProgress(int, uint32_t, uint32_t) {
		return GIT_PASSTHROUGH;
	}
	virtual int sidebandProgress(const std::string_view &) {
		return GIT_PASSTHROUGH;
	}
	virtual int transferProgress(const git_indexer_progress &) {
		return GIT_PASSTHROUGH;
	}
	virtual int updateRefs(const std::string &, const git_oid &, const git_oid &,
			       git_refspec &) {
		return GIT_PASSTHROUGH;
	}
};

}

#endif
