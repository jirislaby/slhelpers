// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>

#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "helpers/String.h"
#include "kerncvs/CollectConfigs.h"

using namespace SlKernCVS;

bool CollectConfigs::collectConfigs(const SlGit::Commit &commit) noexcept
{
	auto tree = commit.tree();

	auto configTreeEntry = tree->treeEntryByPath("config/");
	if (!configTreeEntry)
		return false;
	if (configTreeEntry->type() != GIT_OBJECT_TREE)
		return false;

	auto configTree = repo.treeLookup(*configTreeEntry);
	if (!configTree)
		return false;

	return configTree->walk([this](const std::string &root,
				const SlGit::TreeEntry &entry) -> int {
		if (entry.type() != GIT_OBJECT_BLOB)
			return 0;
		auto flavor = entry.name();
		if (flavor == "vanilla")
			return 0;
		if (!processFlavor(root.substr(0, root.size() - 1), std::move(flavor), entry))
			return -1;
		return 0;
	});
}

bool CollectConfigs::processFlavor(std::string &&arch, std::string &&flavor,
				   const SlGit::TreeEntry &treeEntry)
{
	auto config = treeEntry.catFile(repo);
	if (!config)
		return false;

	return processConfigFile(std::move(arch), std::move(flavor), *config);
}

bool CollectConfigs::processConfigFile(std::string &&arch, std::string &&flavor,
				       std::string_view configFile)
{
	SlHelpers::GetLine gl(configFile);
	auto &map = m_archs[std::move(arch)][std::move(flavor)];
	while (auto line = gl.get())
		if (!processConfig(map, *line))
			return false;

	return true;
}

bool CollectConfigs::processConfig(ConfigMap &map, std::string_view line)
{
	static constexpr const std::string_view commented("# CONFIG_");

	if (line.starts_with(commented)) {
		const auto end = line.find(" is not set");
		if (end == std::string::npos) {
			std::cerr << __func__ <<
				     "commented config without proper 'is not set' in: " <<
				     line << '\n';
			return false;
		}

		std::string config(line.substr(2, end - 2));

		map.emplace(std::move(config), Disabled);
	}
	if (line.starts_with("CONFIG_")) {
		const auto end = line.find('=');
		if (end == std::string::npos) {
			std::cerr << __func__ << "value of config cannot be identified in: " <<
				     line << '\n';
			return false;
		}
		std::string config(line.substr(0, end));
		ConfigValue value = WithValue;
		if (line[end + 1] == 'y')
			value = BuiltIn;
		else if (line[end + 1] == 'm')
			value = Module;
		map.emplace(std::move(config), value);
	}

	return true;
}
