#include <fnmatch.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "helpers/String.h"
#include "kerncvs/SupportedConf.h"

using namespace SlKernCVS;

void SupportedConf::parseLine(std::string &line)
{
	auto vec = SlHelpers::String::split(line, " \t", '#');
	if (vec.empty())
		return;

	auto supp = SupportState::Unspecified;
	if (vec.size() >= 2) {
		const auto &suppFlag = vec[0];
		switch (suppFlag[0]) {
		case '+':
			if (SlHelpers::String::endsWith(suppFlag, "-kmp"))
				supp = SupportState::KMPSupported;
			else if (suppFlag == "+external")
				supp = SupportState::ExternallySupported;
			else if (suppFlag == "+base")
				supp = SupportState::BaseSupported;
			else
				supp = SupportState::Supported;
			break;
		case '-':
			supp = SupportState::Unsupported;
			break;
		default:
			std::cerr << __func__ << ": bad vec from: " << line << "\n";
			return;
		}
	}

	const auto &module = vec.back();
	entries.push_back({ module, supp });
}

SupportedConf::SupportedConf(const std::string &conf)
{
	std::istringstream iss { conf };
	std::string line;

	while (std::getline(iss, line))
		parseLine(line);
}

SupportedConf::SupportState SupportedConf::supportState(const std::string &module) const
{
	for (const auto &e : entries)
		if (!::fnmatch(e.first.c_str(), module.c_str(), FNM_NOESCAPE | FNM_PERIOD))
			return e.second;

	return SupportState::Unsupported;

}
