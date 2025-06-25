#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#include "kerncvs/SupportedConf.h"

using namespace SlKernCVS;

void SupportedConf::parseLine(std::string &line, SupportedConfMap &map)
{
	static const char delim[] = " \t";
	std::vector<std::string> vec;
	auto tok = strtok(line.data(), delim);
	while (tok) {
		if (tok[0] == '#')
			break;
		vec.push_back(tok);
		tok = strtok(nullptr, delim);
	}

	if (vec.empty())
		return;

	const auto &module = vec.back();
	int supp = 0;
	if (vec.size() >= 2) {
		switch (vec[0][0]) {
		case '+':
			supp = 1;
			break;
		case '-':
			supp = -1;
			break;
		default:
			std::cerr << __func__ << ": bad vec from: " << line << "\n";
			return;
		}
	}

	map.insert({ module, supp });
}

SupportedConf::SupportedConfMap SupportedConf::parseSupportedConf(const std::string &conf)
{
	SupportedConf::SupportedConfMap map;
	std::istringstream iss { conf };
	std::string line;

	while (std::getline(iss, line)) {
		if (line.empty() || line[0] == '#')
			continue;
		parseLine(line, map);
	}

	return map;
}
