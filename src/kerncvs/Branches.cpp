#include <regex>
#include <set>
#include <sstream>

#include "kerncvs/Branches.h"
#include "curl/Curl.h"

using namespace SlKernCVS;

Branches::BranchesList Branches::getBuildBranches(const std::string &branchesConf)
{
	std::istringstream iss { branchesConf };

	static std::regex branchesRegex { "^([^#].*):.*\\bbuild\\b" };
	std::string line;
	BranchesList branches;

	while (std::getline(iss, line)) {
		std::smatch m;
		if (!std::regex_search(line, m, branchesRegex))
			continue;

		if (isExcluded(m[1]))
			continue;

		branches.push_back(m[1]);
	}

	return branches;
}

std::optional<Branches::BranchesList> Branches::getBuildBranches()
{
	auto branchesConf = SlCurl::LibCurl::singleDownload("https://kerncvs.suse.de/branches.conf");
	if (!branchesConf)
		return {};

	return getBuildBranches(*branchesConf);
}

bool Branches::isExcluded(const std::string &branch)
{
	static const std::set<std::string> excludes {
		"master",
		"vanilla",
		"linux-next",
		"stable",
		"slowroll",
	};

	return excludes.find(branch) != excludes.end();
}
