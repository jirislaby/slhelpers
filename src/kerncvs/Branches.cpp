#include <regex>
#include <set>
#include <sstream>

#include "kerncvs/Branches.h"
#include "curl/Curl.h"

using namespace SlKernCVS;

std::optional<Branches::BranchesList> Branches::getBuildBranches()
{
	auto branchesConf = SlCurl::LibCurl::singleDownload("https://kerncvs.suse.de/branches.conf");
	if (!branchesConf)
		return {};

	std::istringstream iss { *branchesConf };

	std::regex branchesRegex { "^([^#].*):.*\\bbuild\\b" };
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
