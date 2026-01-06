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

class CollectConfigs {
public:
	enum ConfigValue : char {
		Disabled = 'n',
		BuiltIn = 'y',
		Module = 'm',
		WithValue = 'v',
	};

	using InsertArchFlavor = std::function<int (const std::string &, const std::string &)>;
	using InsertConfig = std::function<int (const std::string &, const std::string &,
		const std::string &, const ConfigValue &)>;

	CollectConfigs(const SlGit::Repo &repo, const InsertArchFlavor &insertArchFlavor,
		       const InsertConfig &insertConfig) : repo(repo),
		insertArchFlavor(insertArchFlavor), insertConfig(insertConfig) {}

	bool collectConfigs(const SlGit::Commit &commit);

private:
	bool processFlavor(const std::string &arch, const std::string &flavor,
			   const SlGit::TreeEntry &treeEntry);
	bool processConfigFile(const std::string &arch, const std::string &flavor,
			       std::string_view configFile);
	bool processConfig(const std::string &arch, const std::string &flavor,
			   std::string_view line);
	const SlGit::Repo &repo;
	const InsertArchFlavor insertArchFlavor;
	const InsertConfig insertConfig;
};

}
