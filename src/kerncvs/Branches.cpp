// SPDX-License-Identifier: GPL-2.0-only

#include <set>
#include <sstream>

#include "curl/Curl.h"
#include "helpers/Color.h"
#include "helpers/String.h"
#include "kerncvs/Branches.h"

using namespace SlKernCVS;
using Clr = SlHelpers::Color;

Branches::BranchesList Branches::getBuildBranches(const std::string &branchesConf)
{
	return Branches::create(branchesConf).filter(BUILD, EXCLUDED);
}

std::optional<Branches::BranchesList> Branches::getBuildBranches()
{
	const auto branchesOpt = Branches::create();
	if (!branchesOpt)
		return std::nullopt;
	return branchesOpt->filter(BUILD, EXCLUDED);
}

Branches Branches::create(const std::string &branchesConf)
{
	std::istringstream iss { branchesConf };

	std::string line;
	BranchesMap branches;

	while (std::getline(iss, line)) {
		const auto split = SlHelpers::String::split(line, " \t", '#');
		if (split.empty())
			continue;

		if (split[0][split[0].size() - 1] != ':') {
			Clr(std::cerr, Clr::RED) << "bad line: " << line;
			continue;
		}

		std::string name(split[0].data(), split[0].size() - 1);
		BranchProps bp{};
		bp.isExcluded = isExcluded(name);
		for (auto i = 1U; i < split.size(); ++i) {
			if (split[i] == "build")
				bp.isBuild = true;
			else if (split[i] == "publish")
				bp.isPublish = true;
			else if (SlHelpers::String::startsWith(split[i], "merge:")) {
				auto idx = 6;
				if (split[i][idx] == '-')
					idx++;
				bp.merges.push_back(split[i].substr(idx));
			}
		}

		branches.emplace(std::move(name), std::move(bp));
	}

	return Branches(branches);
}

std::optional<Branches> Branches::create()
{
	auto branchesConf = SlCurl::LibCurl::singleDownload("https://kerncvs.suse.de/branches.conf");
	if (!branchesConf)
		return std::nullopt;

	return create(*branchesConf);
}

void Branches::dfs(const std::string &u, BranchesSet &visited) const
{
	const auto it = m_map.find(u);
	if (it == m_map.end())
		return;

	for (auto const &v: it->second.merges)
		if (visited.insert(v).second)
			dfs(v, visited);
}

Branches::BranchesSet Branches::mergesClosure(const std::string &branch) const
{
	BranchesSet visited;

	dfs(branch, visited);

	return visited;
}

Branches::BranchesList Branches::filter(unsigned int include, unsigned int exclude) const
{
	Branches::BranchesList ret;

	for (const auto &b: *this) {
		bool add = false;
		if (include == ANY ||
				((include & BUILD) && b.second.isBuild) ||
				((include & PUBLISH) && b.second.isPublish) ||
				((include & EXCLUDED) && b.second.isExcluded))
			add = true;

		if (((exclude & BUILD) && b.second.isBuild) ||
				((exclude & PUBLISH) && b.second.isPublish) ||
				((exclude & EXCLUDED) && b.second.isExcluded))
			add = false;
		if (add)
			ret.push_back(b.first);
	}

	return ret;
}

bool Branches::isExcluded(const std::string_view &branch)
{
	static const std::set<std::string_view> excludes {
		"master",
		"vanilla",
		"linux-next",
		"stable",
		"slowroll",
	};

	return excludes.find(branch) != excludes.end();
}
