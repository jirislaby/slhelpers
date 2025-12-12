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
		auto split = SlHelpers::String::split(line, " \t", '#');
		if (split.empty())
			continue;

		auto name = std::move(split[0]);
		if (name.back() != ':') {
			Clr(std::cerr, Clr::RED) << "bad line: " << line;
			continue;
		}
		name.pop_back();

		BranchProps bp{};
		bp.isExcluded = isExcluded(name);
		for (auto i = 1U; i < split.size(); ++i) {
			static const std::string mergeStr("merge:");
			auto cur = std::move(split[i]);
			if (cur == "build")
				bp.isBuild = true;
			else if (cur == "publish")
				bp.isPublish = true;
			else if (cur.starts_with(mergeStr)) {
				auto toErase = mergeStr.size();
				if (cur[toErase] == '-')
					toErase++;
				bp.merges.push_back(std::move(cur.erase(0, toErase)));
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
