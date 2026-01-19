// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

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
	static KeyPairs get(const std::string &host) noexcept;
private:
	friend void testKeys();
	static void replace(std::string &path, size_t &pos, std::string_view by) noexcept;
	static Key handleTokens(std::string_view host, std::string path) noexcept;
};

}
