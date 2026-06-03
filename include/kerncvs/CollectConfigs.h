// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
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
	 * @param commit The commit in the KernCVS repository to walk
	 */
	CollectConfigs(const SlGit::Commit &commit);

	/// @brief Deleted copy constructor
	CollectConfigs(const CollectConfigs &) = delete;
	/// @brief Deleted copy assignment operator
	CollectConfigs &operator=(const CollectConfigs &) = delete;

	/// @brief Defaulted move constructor
	CollectConfigs(CollectConfigs &&) = default;
	/// @brief Defaulted move assignment operator
	CollectConfigs &operator=(CollectConfigs &&) = default;

	/**
	 * @brief Create a CollectConfigs from the \p repoPath and \p rev
	 * @param repoPath Path to the KernCVS repository
	 * @param rev The revision in the KernCVS repository to walk
	 */
	static CollectConfigs create(const std::filesystem::path &repoPath,
				     const std::string &rev);

	/// @brief Get the arch map
	const auto &getArchMap() const {
		return m_archs;
	}

	/// @brief Get the flavor map for a given \p arch
	const auto &getFlavorMap(const std::string &arch) const {
		return m_archs.at(arch);
	}

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
	void processFlavor(const SlGit::Repo &repo, std::string &&arch, std::string &&flavor,
			   const SlGit::TreeEntry &treeEntry);
	void processConfigFile(std::string &&arch, std::string &&flavor,
			       std::string_view configFile);
	void processConfig(ConfigMap &map, std::string_view line);

	ArchMap m_archs;
};

}
