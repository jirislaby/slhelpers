// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string_view>

#include <git2.h>

namespace SlGit {

/**
 * @brief Callbacks invoked from Repo::clone() or Remote::fetchRefspecs()
 *
 * See also DefaultFetchCallbacks for the default implementation.
 */
class FetchCallbacks {
public:
	/// @brief Construct FetchCallbacks
	FetchCallbacks() {}

	/**
	 * @brief Called for each path in the tree
	 * @param path Path being checked out
	 * @param completedSteps Count of steps done
	 * @param totalSteps Count of all steps
	 */
	virtual void checkoutProgress([[maybe_unused]] std::string_view path,
				      [[maybe_unused]] size_t completedSteps,
				      [[maybe_unused]] size_t totalSteps) {}

	/**
	 * @brief Fill in credentials
	 * @param cred Output
	 * @param url For which URL
	 * @param usernameFromUrl Username detected in \p url
	 * @param allowedTypes \c GIT_CREDENTIAL_USERNAME, \c GIT_CREDENTIAL_SSH_KEY, ...
	 * @return Zero for success.
	 */
	virtual int credentials([[maybe_unused]] git_credential **cred,
				[[maybe_unused]] std::string_view url,
				[[maybe_unused]] std::optional<std::string_view> usernameFromUrl,
				[[maybe_unused]] unsigned int allowedTypes) {
		return GIT_PASSTHROUGH;
	}
	/**
	 * @brief Called many times when packing objects
	 * @param stage At what stage the packing is (\c GIT_PACKBUILDER_ADDING_OBJECTS, ...)
	 * @param current Currently packed object
	 * @param total Total number of objects
	 * @return Zero for success.
	 */
	virtual int packProgress([[maybe_unused]] int stage, [[maybe_unused]] uint32_t current,
				 [[maybe_unused]] uint32_t total) {
		return GIT_PASSTHROUGH;
	}
	/**
	 * @brief Callback for messages received by the transport
	 * @param str The message from the transport
	 * @return Zero for success.
	 */
	virtual int sidebandProgress([[maybe_unused]] std::string_view str) {
		return GIT_PASSTHROUGH;
	}
	/**
	 * @brief Called many times during download and indexing
	 * @param stats The statistics
	 * @return Zero for success.
	 */
	virtual int transferProgress([[maybe_unused]] const git_indexer_progress &stats) {
		return GIT_PASSTHROUGH;
	}
	/**
	 * @brief Called for each updated reference
	 * @param refname The name of the reference
	 * @param a The original OID
	 * @param b The new OID
	 * @param ref The reference structure
	 * @return Zero for success.
	 */
	virtual int updateRefs([[maybe_unused]] std::string_view refname,
			       [[maybe_unused]] const git_oid &a,
			       [[maybe_unused]] const git_oid &b,
			       [[maybe_unused]] git_refspec &ref) {
		return GIT_PASSTHROUGH;
	}
};

}
