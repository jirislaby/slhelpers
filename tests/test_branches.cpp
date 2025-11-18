// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <iostream>
#include <set>

#include "kerncvs/Branches.h"

using namespace SlKernCVS;

namespace {

void checkBuildSet(const std::set<std::string> &set)
{
	// excluded or non-build
	assert(set.find("master") == set.end());
	assert(set.find("vanilla") == set.end());
	assert(set.find("SLE12-SP4-LTSS") == set.end());
	assert(set.find("scripts") == set.end());

	assert(set.find("SL-16.0-AZURE") != set.end());
	assert(set.find("SL-16.0") != set.end());
	assert(set.find("SLE12-SP5") != set.end());
	assert(set.find("SLE12-SP5-RT") != set.end());
	assert(set.find("cve/linux-5.3-LTSS") != set.end());
}

} // namespace

int main()
{
	static const std::string branchesConf = {
		"master:             build publish	merge:scripts\n"
		"vanilla:            build publish\n"
		"stable:             build publish	merge:scripts merge:-master\n"
		"SL-16.0-AZURE:      build publish	merge:SL-16.0\n"
		"SL-16.0:            build publish	merge:scripts\n"
		"SLE12-SP5:          build		merge:scripts\n"
		"SLE12-SP5-RT:       build		merge:-SLE12-SP5\n"
		"SLE12-SP4-LTSS:\n"
		"cve/linux-5.3-LTSS:      build	merge:scripts\n"
		"scripts:                  publish\n"
	};
	{
		const auto branches = Branches::getBuildBranches(branchesConf);

		for (const auto &b : branches)
			std::cerr << b << '\n';

		checkBuildSet({ branches.begin(), branches.end() });
	}
	const auto branches = Branches::create(branchesConf);
	{
		const auto build = branches.filter(Branches::BUILD);
		checkBuildSet({ build.begin(), build.end() });
	}
	{
		const auto excluded = branches.filter(Branches::EXCLUDED, 0);
		std::set<std::string> set { excluded.begin(), excluded.end() };

		assert(set.find("master") != set.end());
		assert(set.find("vanilla") != set.end());

		assert(set.find("SLE12-SP4-LTSS") == set.end());
	}
	{
		const auto nonBuild = branches.filter(Branches::ANY, Branches::BUILD);
		std::set<std::string> set { nonBuild.begin(), nonBuild.end() };

		assert(set.find("scripts") != set.end());
		assert(set.find("SLE12-SP4-LTSS") != set.end());

		assert(set.find("master") == set.end());
		assert(set.find("SL-16.0") == set.end());
	}
	{
		auto merges = branches.merges("stable");
		std::set<std::string> set { merges.begin(), merges.end() };
		assert(set.size() == 2);
		assert(set.find("scripts") != set.end());
		assert(set.find("master") != set.end());
	}

	{
		const auto set = branches.mergesClosure("SLE12-SP5-RT");
		assert(set.size() == 2);
		assert(set.find("SLE12-SP5") != set.end());
		assert(set.find("scripts") != set.end());
	}

	return 0;
}

