// SPDX-License-Identifier: GPL-2.0-only

#ifndef COLLECTCONFIGS_H
#define COLLECTCONFIGS_H

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

	int collectConfigs(const SlGit::Commit &commit);

private:
	int processFlavor(const std::string &arch, const std::string &flavor,
			  const SlGit::TreeEntry &treeEntry);
	int processConfigFile(const std::string &arch, const std::string &flavor,
			      const std::string &configFile);
	int processConfig(const std::string &arch, const std::string &flavor,
			  const std::string &line);
	const SlGit::Repo &repo;
	const InsertArchFlavor insertArchFlavor;
	const InsertConfig insertConfig;
	//static const std::regex REInvalRef;
};

}

#endif

