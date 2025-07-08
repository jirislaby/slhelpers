#include <cassert>
#include <iostream>
#include <set>

#include "kerncvs/Branches.h"

using namespace SlKernCVS;

int main()
{
	static const std::string branchesConf = {
		"master:             build publish	merge:scripts\n"
		"vanilla:            build publish\n"
		"SL-16.0-AZURE:      build publish	merge:SL-16.0\n"
		"SL-16.0:            build publish	merge:scripts\n"
		"SLE12-SP5:          build		merge:scripts\n"
		"SLE12-SP5-RT:       build		merge:-SLE12-SP5\n"
		"SLE12-SP4-LTSS:\n"
		"cve/linux-5.3-LTSS:      build	merge:scripts\n"
		"scripts:                  publish\n"
	};
	auto branches = Branches::getBuildBranches(branchesConf);

	for (const auto &b : branches)
		std::cout << b << '\n';

	std::set<std::string> set { branches.begin(), branches.end() };
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

	return 0;
}

