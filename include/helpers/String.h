// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_STRING_H
#define SLHELPERS_STRING_H

#include <algorithm>
#include <cstring>
#include <cctype>
#include <optional>
#include <string>
#include <utility>
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

	/**
	 * @brief Like string::find() but ignoring case
	 * @param str string to search in
	 * @param sub string to search for
	 * @return position of \p sub in \p str if found, std::string_view::npos otherwise
	 */
	static constexpr std::string_view::size_type
	iFind(const std::string_view &str, const std::string_view &sub) {
		if (str.empty() && sub.empty())
			return 0;
		const auto it = std::search(str.begin(), str.end(), sub.begin(), sub.end(),
				   [](char ch1, char ch2) {
			return std::tolower(static_cast<unsigned char>(ch1)) ==
					      std::tolower(static_cast<unsigned char>(ch2));
		});
		if (it == str.end())
			return std::string_view::npos;
		return it - str.begin();
	}

	template <typename T1, typename T2>
	static constexpr std::string_view::size_type iFind(const T1 &str, const T2 &sub) {
		return iFind(std::string_view(str), std::string_view(sub));
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

	template <typename T>
	static void join(std::ostream &out, const T &iterable,
			 const std::string_view &sep = ", ",
			 const std::string_view &quote = "") {
		bool first = true;
		for (const auto &e: iterable) {
			if (!first)
			    out << sep;
			first = false;

			out << quote << e << quote;
		}
	}
};

}

#endif
