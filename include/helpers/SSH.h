// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "HomeDir.h"

namespace SlSSH {

/**
 * @brief Get SSH private and public keys
 */
class Keys {
public:
	Keys() = delete;

	/// @brief One SSH key (a path to it)
	using Key = std::filesystem::path;
	/// @brief A pair of public + private keys (in that order)
	using KeyPair = std::pair<Key, Key>;
	/// @brief A list of key pairs
	using KeyPairs = std::vector<KeyPair>;

	/**
	 * @brief Get keys for a \p host
	 * @param host Host to get keys for (currently ignored)
	 * @return Private + public keys for \p host.
	 */
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
