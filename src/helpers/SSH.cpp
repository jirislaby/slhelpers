// SPDX-License-Identifier: GPL-2.0-only

#define SSH_NO_CPP_EXCEPTIONS
#include <libssh/libsshpp.hpp>

#include "helpers/HomeDir.h"
#include "helpers/PtrStore.h"
#include "helpers/SSH.h"

using namespace SlSSH;

Keys::KeyPairs Keys::get(const std::string &host) noexcept
{
	ssh::Session session;
	session.setOption(SSH_OPTIONS_HOST, host.c_str());

	if (session.optionsParseConfig(nullptr) != SSH_OK)
		return {};

	SlHelpers::PtrStore<char, decltype([](char *p) { free(p); })> path;
	if (ssh_options_get(session.getCSession(), SSH_OPTIONS_IDENTITY, path.ptr()) != SSH_OK)
		return {};
	if (!path)
		return {};

	auto priv = handleTokens(host, path.get());
	auto pub = priv;
	pub += ".pub";
	if (!std::filesystem::exists(priv) || !std::filesystem::exists(pub))
		return {};
	return { { pub, priv } };
}

Keys::Key Keys::handleTokens(std::string_view host, std::string path) noexcept
{
	if (path.empty())
		return path;

	auto home = SlHelpers::HomeDir::get().string();

	if (path.starts_with("~/"))
		path.replace(0, 1, home);

	size_t pos = 0;
	while ((pos = path.find("%", pos)) != std::string::npos) {
		if (pos + 1 >= path.length())
			break;
		switch (path[pos + 1]) {
		case '%':
			path.erase(pos, 1);
			pos++;
			break;
		case 'd':
			replace(path, pos, home);
			break;
		case 'h':
			replace(path, pos, host);
			break;
		default:
			pos += 2;
			break;
		}
	}

	return path;
}

void Keys::replace(std::string &path, size_t &pos, std::string_view by) noexcept
{
	path.replace(pos, 2, by);
	pos += by.length();
}
