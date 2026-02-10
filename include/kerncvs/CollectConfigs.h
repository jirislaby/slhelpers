// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <functional>
#include <string>

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

	/// @brief A callback invoked for arch and flavor
	using InsertArchFlavor = std::function<int (const std::string &, const std::string &)>;
	/// @brief A callback invoked for arch, flavor, config, and its value
	using InsertConfig = std::function<int (const std::string &, const std::string &,
		const std::string &, const ConfigValue &)>;

	/**
	 * @brief CollectConfigs constructor
	 * @param repo KernCVS repository to search in
	 * @param insertArchFlavor Callback to invoke for an arch and flavor
	 * @param insertConfig Callback to invoke for a config
	 */
	CollectConfigs(const SlGit::Repo &repo, const InsertArchFlavor &insertArchFlavor,
		       const InsertConfig &insertConfig) : repo(repo),
		insertArchFlavor(insertArchFlavor), insertConfig(insertConfig) {}

	/**
	 * @brief The real work function of this class
	 * @param commit The commit to walk
	 * @return true on success.
	 */
	bool collectConfigs(const SlGit::Commit &commit) noexcept;

private:
	bool processFlavor(const std::string &arch, const std::string &flavor,
			   const SlGit::TreeEntry &treeEntry) noexcept;
	bool processConfigFile(const std::string &arch, const std::string &flavor,
			       std::string_view configFile) noexcept;
	bool processConfig(const std::string &arch, const std::string &flavor,
			   std::string_view line) noexcept;
	const SlGit::Repo &repo;
	const InsertArchFlavor insertArchFlavor;
	const InsertConfig insertConfig;
};

}
