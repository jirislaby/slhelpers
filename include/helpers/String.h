// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_STRING_H
#define SLHELPERS_STRING_H

#include <algorithm>
#include <cstring>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

namespace SlHelpers {

/**
 * @brief Parses a string view into lines.
 *
 * Use like:
 * @code
 * GetLine g("str");
 * while (auto line = g.get()) {}
 * @endcode
 */
class GetLine {
public:
	GetLine(std::string_view str) noexcept : m_str(str) {}

	/**
	 * @brief Read one line
	 * @return Line if one was read, otherwise nullopt.
	 */
	std::optional<std::string_view> get() noexcept {
		if (m_str.empty())
			return std::nullopt;

		const auto eol = m_str.find('\n');
		const auto last = eol == std::string_view::npos;
		auto line = last ? m_str : m_str.substr(0, eol);

		m_str.remove_prefix(last ? m_str.size() : eol + 1);

		return line;
	}
private:
	std::string_view m_str;
};

class String {
public:
	inline static constinit const auto npos = std::string_view::npos;
	static_assert(std::string_view::npos == std::string::npos);

	String() = delete;

	/**
	 * @brief Like string::find() but ignoring case
	 * @param str string to search in
	 * @param sub string to search for
	 * @return position of \p sub in \p str if found, npos otherwise
	 */
	static constexpr std::string_view::size_type
	iFind(std::string_view str, std::string_view sub) noexcept {
		if (str.empty() && sub.empty())
			return 0;
		const auto it = std::search(str.begin(), str.end(), sub.begin(), sub.end(),
				   [](char ch1, char ch2) {
			return std::tolower(static_cast<unsigned char>(ch1)) ==
					      std::tolower(static_cast<unsigned char>(ch2));
		});
		if (it == str.end())
			return npos;
		return it - str.begin();
	}

	static std::vector<std::string>
	split(std::string str, const std::string &delim,
	      const std::optional<char> &comment = std::nullopt) noexcept {
		std::vector<std::string> res;

		auto tok = ::strtok(str.data(), delim.c_str());
		while (tok) {
			if (comment && tok[0] == *comment)
				break;
			res.push_back(tok);
			tok = ::strtok(nullptr, delim.c_str());
		}

		return res;
	}

	/**
	 * @brief Split \p str by \p delim into a vector, ignoring everything after \p comment
	 * @param str String to parse
	 * @param delim Delimeter to split by
	 * @param comment Ignore string after character
	 * @return Vector built of \p str.
	 *
	 * Note \p str must outlive the use of the return value!
	 */
	static constexpr std::vector<std::string_view>
	splitSV(std::string_view str,
		std::string_view delim,
		std::optional<char> comment = std::nullopt) noexcept
	{
		std::vector<std::string_view> res;
		std::size_t end = 0;

		do {
			const auto start = str.find_first_not_of(delim, end);
			if (start == npos)
				break;

			end = str.find_first_of(delim, start);
			const auto len = (end == npos) ? (str.length() - start) : (end - start);
			auto token = str.substr(start, len);
			if (comment && !token.empty() && token[0] == *comment)
				break;

			res.push_back(token);
		} while (end != npos);

		return res;
	}

	template <typename T>
	static constexpr T trim(const T &line) noexcept
	{
		constexpr const std::string_view spaces{" \n\t\r"};
		const auto pos1 = line.find_first_not_of(spaces);
		const auto pos2 = line.find_last_not_of(spaces);

		if (pos1 == npos)
			return {};

		return line.substr(pos1, pos2 - pos1 + 1);
	}

	static constexpr bool isHex(std::string_view s) noexcept {
		return std::all_of(s.cbegin(), s.cend(), ::isxdigit);
	}

	template <typename T>
	static constexpr void join(std::ostream &out, const T &iterable,
				   std::string_view sep = ", ",
				   std::string_view quote = "") noexcept {
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
