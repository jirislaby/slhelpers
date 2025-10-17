// SPDX-License-Identifier: GPL-2.0-only

#ifndef STRING_H
#define STRING_H

#include <algorithm>
#include <cstring>
#include <ctype.h>
#include <optional>
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

	static std::vector<std::string> split(const std::string &str, const std::string &delim,
					      const std::optional<char> &comment = std::nullopt) {
		std::vector<std::string> res;
		std::string copy(str);

		auto tok = ::strtok(copy.data(), delim.c_str());
		while (tok) {
			if (comment && tok[0] == *comment)
				break;
			res.push_back(tok);
			tok = ::strtok(nullptr, delim.c_str());
		}

		return res;
	}

	template <typename T>
	static T trim(const T &line)
	{
		static constexpr const std::string_view spaces{" \n\t\r"};
		const auto pos1 = line.find_first_not_of(spaces);
		const auto pos2 = line.find_last_not_of(spaces);

		if (pos1 == std::string::npos)
			return {};

		return line.substr(pos1, pos2 - pos1 + 1);
	}

	static bool isHex(const std::string_view &s) {
		return std::all_of(s.cbegin(), s.cend(), ::isxdigit);
	}
};

}

#endif
