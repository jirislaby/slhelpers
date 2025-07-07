// SPDX-License-Identifier: GPL-2.0-only

#ifndef SSH_H
#define SSH_H

#include <string>
#include <vector>

#include "HomeDir.h"

namespace SlSSH {

class Keys {
public:
	Keys() = delete;

	using Key = std::filesystem::path;
	// public, private
	using KeyPair = std::pair<Key, Key>;
	using KeyPairs = std::vector<KeyPair>;

	static KeyPairs get(const std::string &host) {
		(void)host;
		auto home = SlHelpers::HomeDir::get();
		auto sshDir = home / ".ssh";
		KeyPairs res;

		for (const auto &dirEntry : std::filesystem::directory_iterator { sshDir }) {
			const auto &pub = dirEntry.path();

			if (!dirEntry.is_regular_file() || pub.extension() != ".pub")
				continue;

			auto priv {pub};
			priv.replace_extension();
			if (std::filesystem::exists(priv))
				res.push_back(std::make_pair(pub, std::move(priv)));
		}

		return res;
	}
};

}

#endif
