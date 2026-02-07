// SPDX-License-Identifier: GPL-2.0-only

#include <set>

#include "curl/Curl.h"
#include "helpers/Color.h"
#include "helpers/String.h"
#include "kerncvs/Branches.h"

using namespace SlKernCVS;
using Clr = SlHelpers::Color;

Branches::BranchesList Branches::getBuildBranches(std::string_view branchesConf) noexcept
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

std::chrono::year_month_day Branches::parseDate(std::string_view date)
{
	if (date.length() != 10 || date[4] != '-' || date[7] != '-')
		return {};

	auto year = SlHelpers::String::toNum(date.substr(0, 4));
	auto month =  SlHelpers::String::toNum(date.substr(5, 2));
	auto day =  SlHelpers::String::toNum(date.substr(8, 2));
	if (!year || !month || !day)
		return {};

	return { std::chrono::year(*year), std::chrono::month(*month), std::chrono::day(*day) };
}

Branches Branches::create(std::string_view branchesConf) noexcept
{
	BranchesMap branches;

	SlHelpers::GetLine gl(branchesConf);
	while (auto line = gl.get()) {
		auto split = SlHelpers::String::splitSV(*line, " \t", '#');
		if (split.empty())
			continue;

		auto name = split[0];
		if (name.back() != ':') {
			Clr(std::cerr, Clr::RED) << "bad line: " << *line;
			continue;
		}
		name.remove_suffix(1);

		BranchProps bp{};
		bp.isExcluded = isExcluded(name);
		for (auto i = 1U; i < split.size(); ++i) {
			static constinit const std::string_view mergeStr("merge:");
			static constinit const std::string_view eolStr("eol:");
			auto cur = split[i];
			if (cur == "build")
				bp.isBuild = true;
			else if (cur == "publish")
				bp.isPublish = true;
			else if (cur.starts_with(mergeStr)) {
				cur.remove_prefix(mergeStr.size());
				if (!cur.empty() && cur.front() == '-')
					cur.remove_prefix(1);
				bp.merges.emplace_back(cur);
			} else if (cur.starts_with(eolStr)) {
				cur.remove_prefix(eolStr.size());
				bp.eol = parseDate(cur);
			}
		}

		branches.emplace(name, std::move(bp));
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

void Branches::dfs(std::string_view u, BranchesSet &visited) const
{
	const auto it = m_map.find(u);
	if (it == m_map.end())
		return;

	for (auto const &v: it->second.merges)
		if (visited.insert(v).second)
			dfs(v, visited);
}

Branches::BranchesSet Branches::mergesClosure(std::string_view branch) const
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

bool Branches::isExcluded(std::string_view branch)
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
