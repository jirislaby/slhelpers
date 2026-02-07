// SPDX-License-Identifier: GPL-2.0-only

#pragma once

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
	/**
	 * @brief Construct GetLine to parse \p str
	 * @param str String to parse
	 */
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

/**
 * @brief Various helpers on strings
 */
class String {
public:
	/// @brief A local alias for std::string_view::npos
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

	/**
	 * @brief Split a string into vector of strings, DO NOT USE THIS
	 * @param str
	 * @param delim
	 * @param comment
	 * @return
	 */
	[[deprecated]] static std::vector<std::string>
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
	static std::vector<std::string_view>
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

	/**
	 * @brief Trim string (remove surrounding whitespace)
	 * @param line String to trim
	 * @return Trimmed string.
	 */
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

	/**
	 * @brief Is the string consisting of hex number?
	 * @param s String to inspect
	 * @return true if all characters are hex numbers
	 */
	static constexpr bool isHex(std::string_view s) noexcept {
		return std::all_of(s.cbegin(), s.cend(), ::isxdigit);
	}

	/**
	 * @brief Join \p iterable into \p out using separator \p sep and quoting \p quote
	 * @param out Output stream
	 * @param iterable Input container
	 * @param sep Separator
	 * @param quote Quoting string, it is put before and after each value in \p iterable
	 */
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


	/**
	 * @brief Hash for string and string_view to be used in hashing containers
	 *
	 * It is to support find() to work on both string and string_view.
	 */
	struct Hash {
		/// @brief For containers to know this is a transparent hash
		using is_transparent = void;

		/**
		 * @brief Hash any kind of string using std::hash
		 * @param sv String to hash
		 */
		auto hash(std::string_view sv) const noexcept {
			return std::hash<std::string_view>{}(sv);
		}

		/// @brief Hash \p charp
		size_t operator()(const char *charp) const noexcept { return hash(charp); }

		/// @brief Hash string_view \p sv
		size_t operator()(std::string_view sv) const noexcept { return hash(sv); }

		/// @brief Hash string \p s
		size_t operator()(const std::string &s) const noexcept { return hash(s); }
	};

	/**
	 * @brief Equality test for string and string_view to be used in containers
	 *
	 * It is to support find() to work on both string and string_view.
	 */
	struct Eq {
		/// @brief For containers to know this is a transparent eq test
		using is_transparent = void;

		/**
		 * @brief Compare any kinds of two strings \p a and \p b
		 * @param a First string
		 * @param b Second string
		 * @return true if \p a == \p b
		 */
		bool operator()(std::string_view a, std::string_view b) const noexcept {
			return a == b;
		}
	};
};

}
