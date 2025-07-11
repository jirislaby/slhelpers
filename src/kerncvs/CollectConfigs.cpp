// SPDX-License-Identifier: GPL-2.0-only

#include "kerncvs/CollectConfigs.h"
#include "git/Git.h"
#include <iostream>

using namespace SlKernCVS;

 int CollectConfigs::collectConfigs(const SlGit::Commit &commit)
{
	SlGit::Tree tree;
	tree.ofCommit(commit);

	SlGit::TreeEntry configTreeEntry;
	if (configTreeEntry.byPath(tree, "config/"))
		return -1;
	if (configTreeEntry.type() != GIT_OBJECT_TREE)
		return -1;

	SlGit::Tree configTree;
	if (configTree.lookup(repo, configTreeEntry))
		return -1;
	std::cout << __func__ << ": t=" << configTree.idStr() << '\n';
	auto ret = configTree.walk([this](const std::string &root,
				   const SlGit::TreeEntry &entry) -> int {
		//std::cerr << "walk: " << root << entry.name() << " type=" << entry.type() << " id=" << entry.idStr() << '\n';
		if (entry.type() == GIT_OBJECT_BLOB)
			return processFlavor(root.substr(0, root.size() - 1), entry.name(), entry);

		return 0;
	});
	if (ret) {
		return -1;
	}

	return 0;
}

 int CollectConfigs::processFlavor(const std::string &arch, const std::string &flavor,
				   const SlGit::TreeEntry &treeEntry)
 {
	std::cout << __func__ << ": a=" << arch << " f=" << flavor << " entry=" <<
		     treeEntry.idStr() << '\n';

	auto config = treeEntry.catFile(repo);
	if (!config)
		return -1;
	std::cout << '\t' << config->substr(0, 100) << '\n';

	return 0;
 }
