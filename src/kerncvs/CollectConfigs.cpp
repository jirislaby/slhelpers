// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>

#include "helpers/String.h"
#include "kerncvs/CollectConfigs.h"
#include "git/Git.h"

using namespace SlKernCVS;

 int CollectConfigs::collectConfigs(const SlGit::Commit &commit)
{
	auto tree = commit.tree();

	auto configTreeEntry = tree->treeEntryByPath("config/");
	if (!configTreeEntry)
		return -1;
	if (configTreeEntry->type() != GIT_OBJECT_TREE)
		return -1;

	auto configTree = repo.treeLookup(*configTreeEntry);
	if (!configTree)
		return -1;

	auto ret = configTree->walk([this](const std::string &root,
				    const SlGit::TreeEntry &entry) -> int {
		if (entry.type() == GIT_OBJECT_BLOB)
			return processFlavor(root.substr(0, root.size() - 1), entry.name(), entry);

		return 0;
	});
	return ret;
}

int CollectConfigs::processFlavor(const std::string &arch, const std::string &flavor,
				  const SlGit::TreeEntry &treeEntry)
{
	auto config = treeEntry.catFile(repo);
	if (!config)
		return -1;

	return processConfigFile(arch, flavor, *config);
}

int CollectConfigs::processConfigFile(const std::string &arch, const std::string &flavor,
				      const std::string &configFile)
{
	if (insertArchFlavor(arch, flavor))
		return -1;

	std::istringstream iss { configFile };
	std::string line;

	while (std::getline(iss, line))
		if (processConfig(arch, flavor, line))
			return -1;

	return 0;
}

int CollectConfigs::processConfig(const std::string &arch, const std::string &flavor,
				   const std::string &line)
{
	static const std::string commented{"# CONFIG_"};

	if (SlHelpers::String::startsWith(line, commented)) {
		const auto end = line.find(" is not set");
		if (end == std::string::npos) {
			std::cerr << __func__ <<
				     "commented config without proper 'is not set' in: " <<
				     line << '\n';
			return -1;
		}

		const auto config = line.substr(2, end - 2);

		return insertConfig(arch, flavor, config, Disabled);

	}
	if (SlHelpers::String::startsWith(line, "CONFIG_")) {
		const auto end = line.find('=');
		if (end == std::string::npos) {
			std::cerr << __func__ << "value of config cannot be identified in: " <<
				     line << '\n';
			return -1;
		}
		const auto config = line.substr(0, end);
		ConfigValue value = WithValue;
		if (line[end + 1] == 'y')
			value = BuiltIn;
		else if (line[end + 1] == 'm')
			value = Module;
		return insertConfig(arch, flavor, config, value);
	}

	return 0;
}
