// SPDX-License-Identifier: GPL-2.0-only

#include <iomanip>

#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "helpers/String.h"
#include "helpers/Exception.h"
#include "kerncvs/CollectConfigs.h"

using RunEx = SlHelpers::RuntimeException;
using SlHelpers::raise;

using namespace SlKernCVS;

CollectConfigs CollectConfigs::create(const std::filesystem::path &repoPath,
				      const std::string &rev)
{
	auto repo = SlGit::Repo::open(repoPath);
	if (!repo)
		RunEx("Failed to open repository at ") << repoPath << ": " <<
			repo->lastError() << raise;

	auto commit = repo->commitRevparseSingle(rev);
	if (!commit)
		RunEx("Failed to find commit for revision ") << std::quoted(rev) << ": " <<
			repo->lastError() << raise;

	return CollectConfigs(*commit);
}

CollectConfigs::CollectConfigs(const SlGit::Commit &commit)
{
	auto &repo = commit.repo();
	auto tree = commit.tree();

	auto configTreeEntry = tree->treeEntryByPath("config/");
	if (!configTreeEntry)
		RunEx("config/ not found in commit ") << std::quoted(commit.idStr()) << ": " <<
			repo.lastError() << raise;
	if (configTreeEntry->type() != GIT_OBJECT_TREE)
		RunEx("config/ is not a tree in commit ") << std::quoted(commit.idStr()) << raise;

	auto configTree = repo.treeLookup(*configTreeEntry);
	if (!configTree)
		RunEx("config/ not found in commit ") << std::quoted(commit.idStr()) << ": " <<
			repo.lastError() << raise;

	std::string err;

	auto ret = configTree->walk([this, &repo, &err](const std::string &root,
				const SlGit::TreeEntry &entry) -> int {
		if (entry.type() != GIT_OBJECT_BLOB)
			return 0;
		auto flavor = entry.name();
		if (flavor == "vanilla")
			return 0;
		try {
			processFlavor(repo, root.substr(0, root.size() - 1), std::move(flavor),
				      entry);
		} catch (const std::runtime_error &e) {
			err = e.what();
			return -1;
		}
		return 0;
	});

	if (!err.empty())
		RunEx(err).raise();

	if (!ret)
		RunEx("Error while walking config/ tree in commit ") <<
			std::quoted(commit.idStr()) << ": " << repo.lastError() << raise;
}

void CollectConfigs::processFlavor(const SlGit::Repo &repo, std::string &&arch,
				   std::string &&flavor, const SlGit::TreeEntry &treeEntry)
{
	auto config = treeEntry.catFile(repo);
	if (!config)
		RunEx("Failed to read config file for \"") << arch << '/' << flavor << '/' <<
			treeEntry.name() << "\": " << repo.lastError() << raise;

	processConfigFile(std::move(arch), std::move(flavor), *config);
}

void CollectConfigs::processConfigFile(std::string &&arch, std::string &&flavor,
				       std::string_view configFile)
{
	SlHelpers::GetLine gl(configFile);
	auto &map = m_archs[std::move(arch)][std::move(flavor)];
	while (auto line = gl.get())
		processConfig(map, *line);
}

void CollectConfigs::processConfig(ConfigMap &map, std::string_view line)
{
	static constexpr const std::string_view commented("# CONFIG_");

	if (line.starts_with(commented)) {
		const auto end = line.find(" is not set");
		if (end == std::string::npos)
			RunEx(__func__) << ": commented config without proper 'is not set' in: " <<
				std::quoted(line) << raise;

		std::string config(line.substr(2, end - 2));

		map.emplace(std::move(config), Disabled);
	}
	if (line.starts_with("CONFIG_")) {
		const auto end = line.find('=');
		if (end == std::string::npos)
			RunEx(__func__) << ": value of config cannot be identified in: " <<
				     line << raise;

		std::string config(line.substr(0, end));
		ConfigValue value = WithValue;
		if (line[end + 1] == 'y')
			value = BuiltIn;
		else if (line[end + 1] == 'm')
			value = Module;
		map.emplace(std::move(config), value);
	}
}
