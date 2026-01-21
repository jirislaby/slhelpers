// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>

#include "../git/Repo.h"
#include "../helpers/String.h"

namespace SlKernCVS {

/**
 * @brief Parse rpm/config.sh into a map of key -> value
 */
class RPMConfig {
	using ConfigMap = std::unordered_map<std::string, std::string, SlHelpers::String::Hash,
			SlHelpers::String::Eq>;
public:
	RPMConfig() = delete;

	/// @brief Create a new RPMConfig from the \p rpmConfig
	RPMConfig(std::string_view rpmConfig) noexcept {
		SlHelpers::GetLine g(rpmConfig);
		while (auto l = g.get()) {
			auto trimmed = SlHelpers::String::trim(*l);
			if (trimmed.empty() || trimmed.front() == '#')
				continue;
			auto eq = trimmed.find('=');
			if (eq == trimmed.npos)
				continue;
			auto key = SlHelpers::String::trim(trimmed.substr(0, eq));
			auto val = SlHelpers::String::trim(trimmed.substr(eq + 1));
			if (val.size() >= 2 && val.front() == '"' && val.back() == '"')
				val = val.substr(1, val.size() - 2);

			m_config[std::string(key)] = val;
		}
	}

	/// @brief Create a new RPMConfig from the \p branch in the \p repo
	static std::optional<RPMConfig> create(const SlGit::Repo &repo,
					       const std::string &branch) noexcept {
		const auto config = repo.catFile("origin/" + branch, "rpm/config.sh");
		if (!config)
			return std::nullopt;
		return RPMConfig(*config);
	}

	/// @brief Test whether @p key exists in the config
	bool contains(std::string_view key) const noexcept { return m_config.contains(key); }

	/// @brief Find @p key in the config and return its value if found (or nullopt)
	std::optional<std::reference_wrapper<const std::string>>
	get(std::string_view key) const noexcept {
		auto it = m_config.find(key);
		if (it == m_config.end())
			return std::nullopt;
		return std::cref(it->second);
	}

	/// @brief Find @p key in the config and return its value
	const auto &operator[](std::string_view key) const noexcept {
		return m_config.find(key)->second;
	}
private:
	ConfigMap m_config;
};

}
