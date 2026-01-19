// SPDX-License-Identifier: GPL-2.0-only
#include <bitset>
#include <iostream>
#include <pwd.h>
#include <unistd.h>

#include "git/DefaultFetchCallbacks.h"
#include "git/Helpers.h"
#include "helpers/Misc.h"

using namespace SlGit;

namespace {
const constexpr std::string_view clearLine("\33[2K\r");
const constexpr bool do_ratelimit = 1;
}

void DefaultFetchCallbacks::checkoutProgress(std::string_view path, size_t completedSteps,
					     size_t totalSteps)
{
	if (do_ratelimit && completedSteps != 0 && completedSteps != totalSteps &&
			!ratelimit.limit())
		return;
	std::cerr << clearLine << "Checked-out: " << completedSteps << '/' << totalSteps <<
		     " (" << path << ')';
	if (completedSteps == totalSteps)
		std::cerr << '\n';
}

int DefaultFetchCallbacks::credentials(git_credential **out, std::string_view url,
				       std::optional<std::string_view> usernameFromUrl,
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

int DefaultFetchCallbacks::packProgress(int stage, uint32_t current, uint32_t total)
{
	if (!do_ratelimit || current == 0 || current == total || ratelimit.limit())
		std::cerr << "Packing objects: stage=" << stage << " " << current << "/" <<
			     total << '\n';
	return 0;
}

int DefaultFetchCallbacks::sidebandProgress(std::string_view str)
{
	if (!do_ratelimit || ratelimit.limit())
		std::cerr << clearLine << "remote: " << str;
	return 0;
}

int DefaultFetchCallbacks::transferProgress(const git_indexer_progress &stats)
{
	if (stats.received_objects == stats.total_objects && stats.total_deltas) {
		const bool final = stats.indexed_deltas == stats.total_deltas;
		if (!do_ratelimit || final || ratelimit.limit())
			std::cerr << clearLine << "Resolving deltas " << stats.indexed_deltas <<
				     '/' << stats.total_deltas;
		if (final)
			std::cerr << '\n';
	} else if (stats.total_objects > 0) {
		const bool final = stats.received_objects == stats.total_objects;
		if (!do_ratelimit || stats.indexed_objects == 0 || final || ratelimit.limit())
			std::cerr << clearLine << "Received " <<
				     std::fixed << std::setprecision(2) <<
				     stats.received_objects << '/' <<
				     stats.total_objects << " objects (" <<
				     stats.indexed_objects << ") in " <<
				     SlHelpers::Unit::human(stats.received_bytes);
		if (final)
			std::cerr << '\n';
	}
	return 0;
}

int DefaultFetchCallbacks::updateRefs(std::string_view refname, const git_oid &a, const git_oid &b,
				      git_refspec &) {
	const auto b_str = SlGit::Helpers::oidToStr(b);

	if (git_oid_is_zero(&a)) {
		std::cerr << "[new]     " << b_str.substr(0, 20) << ' ' << refname << '\n';
	} else {
		std::cerr << "[updated] " <<
			     SlGit::Helpers::oidToStr(a).substr(0, 10) << ".." <<
			     b_str.substr(0, 10) << ' ' << refname << '\n';
	}
	return 0;
}

std::string DefaultFetchCallbacks::getUserName(std::optional<std::string_view> usernameFromUrl)
{
	if (!userName.empty())
		return userName;

	if (usernameFromUrl)
		return userName = *usernameFromUrl;

	return userName = ::getpwuid(::getuid())->pw_name;
}
