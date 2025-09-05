// SPDX-License-Identifier: GPL-2.0-only
#include <bitset>
#include <iostream>

#include "MyFetchCallbacks.h"

using namespace SlGit;

int MyFetchCallbacks::credentials(git_credential **out, const std::string &url,
				  const std::optional<std::string> &usernameFromUrl,
				  unsigned int allowedTypes)
{
	auto user = getUserName(usernameFromUrl);
	std::cerr << __func__ << ": url=" << url << " user=" << user <<
		     " types=" << std::bitset<8>{allowedTypes} <<
		     " tried=" << std::bitset<8>{tried} <<
		     " keys=" << keys.size() <<
		     " triedKey=" << triedKey << '\n';

	if (allowedTypes & GIT_CREDENTIAL_USERNAME)
		return git_credential_username_new(out, user.c_str());

	if (allowedTypes & GIT_CREDENTIAL_SSH_KEY && !(tried & GIT_CREDENTIAL_SSH_KEY)) {
		if (triedKey >= keys.size()) {
			tried |= GIT_CREDENTIAL_SSH_KEY;
			return GIT_PASSTHROUGH;
		}
		const auto &keyPair = keys[triedKey++];
		return git_credential_ssh_key_new(out, user.c_str(),
						  keyPair.first.string().c_str(),
						  keyPair.second.string().c_str(), nullptr);
	}

	std::cerr << "\tUNHANDLED!\n";

	return GIT_PASSTHROUGH;
}

int MyFetchCallbacks::packProgress(int stage, uint32_t current, uint32_t total)
{
	if (!ratelimit.limit() && current != 0 && current != total)
		return 0;
	std::cerr << __func__ << ": stage=" << stage << " " << current << "/" << total << '\n';
	return 0;
}

int MyFetchCallbacks::sidebandProgress(const std::string_view &str)
{
	if (!ratelimit.limit())
		return 0;
	std::cerr << __func__ << ": " << str;
	if (str.back() != '\n')
		std::cerr << '\n';
	return 0;
}

int MyFetchCallbacks::transferProgress(const git_indexer_progress &stats)
{
	if (!ratelimit.limit() && stats.indexed_objects != 0 &&
			stats.indexed_objects != stats.total_objects)
		return 0;
	std::cerr << std::fixed << std::setprecision(2) << __func__ <<
		     ": deltas=" << stats.indexed_deltas << '/' << stats.total_deltas <<
		     " objs=" << stats.indexed_objects << '/' << stats.total_objects <<
		     " local=" << stats.local_objects <<
		     " recv=" << stats.received_objects <<
		     " (" << stats.received_bytes / 1024. << " kB)\n";
	return 0;
}

int MyFetchCallbacks::updateRefs(const std::string &refname, const git_oid &a, const git_oid &b,
				 git_refspec &) {
	char sha1[12] = {0}, sha2[12] = {0};
	git_oid_tostr(sha1, sizeof(sha1) - 1, &a);
	git_oid_tostr(sha2, sizeof(sha2) - 1, &b);
	std::cerr << __func__ << ": ref=" << refname << " " << sha1 << ".." << sha2 << '\n';
	return 0;
}

std::string MyFetchCallbacks::getUserName(const std::optional<std::string> &usernameFromUrl)
{
	if (!userName.empty())
		return userName;

	if (usernameFromUrl)
		return userName = *usernameFromUrl;

	return userName = ::getpwuid(::getuid())->pw_name;
}
