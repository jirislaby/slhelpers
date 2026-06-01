// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <unordered_map>

#include "../helpers/String.h"

namespace SlGit {
class Commit;
class Repo;
class TreeEntry;
}

namespace SlKernCVS {

/**
 * @brief Class to walk the KernCVS repository and report arch, flavor and configs via callbacks
 * passed to the constructor.
 */
class CollectConfigs {
public:
	/// @brief Value for a config
	enum ConfigValue : char {
		Disabled = 'n',
		BuiltIn = 'y',
		Module = 'm',
		WithValue = 'v',
	};

	/// @brief Map of config name to config value
	using ConfigMap = std::unordered_map<std::string, ConfigValue, SlHelpers::String::Hash,
	      SlHelpers::String::Eq>;
	/// @brief Map of flavor name to config map
	using FlavorMap = std::unordered_map<std::string, ConfigMap, SlHelpers::String::Hash,
	      SlHelpers::String::Eq>;
	/// @brief Map of arch name to flavor map
	using ArchMap = std::unordered_map<std::string, FlavorMap, SlHelpers::String::Hash,
	      SlHelpers::String::Eq>;

	/**
	 * @brief CollectConfigs constructor
	 * @param repo KernCVS repository to search in
	 */
	CollectConfigs(const SlGit::Repo &repo) : repo(repo) {}

	/**
	 * @brief The real work function of this class
	 * @param commit The commit to walk
	 * @return true on success.
	 */
	bool collectConfigs(const SlGit::Commit &commit) noexcept;

	/// @brief Get the config map for a given \p arch, \p flavor
	const auto &getConfigMap(const std::string &arch, const std::string &flavor) const {
		return m_archs.at(arch).at(flavor);
	}

	/// @brief Get the config value for a given \p arch, \p flavor and \p config
	auto getConfig(const std::string &arch, const std::string &flavor,
		       const std::string &config) const {
		return m_archs.at(arch).at(flavor).at(config);
	}

private:
	bool processFlavor(std::string &&arch, std::string &&flavor,
			   const SlGit::TreeEntry &treeEntry);
	bool processConfigFile(std::string &&arch, std::string &&flavor,
			       std::string_view configFile);
	bool processConfig(ConfigMap &map, std::string_view line);
	const SlGit::Repo &repo;

	ArchMap m_archs;
};

}
