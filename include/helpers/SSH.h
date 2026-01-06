// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_SSH_H
#define SLHELPERS_SSH_H

#include <string_view>
#include <utility>
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

	static KeyPairs get([[maybe_unused]] std::string_view host) {
		auto home = SlHelpers::HomeDir::get();
		auto sshDir = home / ".ssh";

		if (!std::filesystem::exists(sshDir))
			return {};

		KeyPairs res;

		for (const auto &dirEntry : std::filesystem::directory_iterator { sshDir }) {
			const auto &pub = dirEntry.path();

			if (!dirEntry.is_regular_file() || pub.extension() != ".pub")
				continue;

			auto priv {pub};
			priv.replace_extension();
			if (std::filesystem::exists(priv))
				res.emplace_back(pub, std::move(priv));
		}

		return res;
	}
};

}

#endif
