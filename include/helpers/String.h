// SPDX-License-Identifier: GPL-2.0-only

#ifndef STRING_H
#define STRING_H

#include <cstring>
#include <string>
#include <vector>

namespace SlHelpers {

class String {
public:
	String() = delete;

	static bool startsWith(const std::string &what, const std::string &startsWith) {
		return !what.compare(0, startsWith.length(), startsWith);
	}

	static bool endsWith(const std::string &what, const std::string &endsWith) {
		auto wlen = what.length();
		auto elen = endsWith.length();
		return wlen >= elen && !what.compare(wlen - elen, std::string::npos, endsWith);
	}

	static std::vector<std::string> split(const std::string &str, const std::string &delim) {
		std::vector<std::string> res;
		std::string copy(str);

		auto tok = ::strtok(copy.data(), delim.c_str());
		while (tok) {
			res.push_back(tok);
			tok = ::strtok(nullptr, delim.c_str());
		}

		return res;
	}

};

}

#endif
