// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <pcre2.h>
#include <string_view>

#include "../helpers/LastError.h"

namespace SlPCRE2 {

/**
 * @brief Iterator over matches
 */
struct MatchIterator {
	MatchIterator() = delete;

	/// @brief Obtain the current match
	auto operator*() const noexcept {
		return matchByIdx(m_ovector, m_subject, m_idx);
	}

	/// @brief Move to the next match
	MatchIterator &operator++() noexcept { ++m_idx; return *this; }
	/// @brief Move to the previous match
	MatchIterator &operator--() noexcept { --m_idx; return *this; }
	/// @brief Move to the next match
	MatchIterator operator++(int) noexcept {
		auto old = *this;
		++m_idx;
		return old;
	}
	/// @brief Move to the previous match
	MatchIterator operator--(int) noexcept {
		auto old = *this;
		--m_idx;
		return old;
	}
	/// @brief Compare two MatchIterators
	auto operator==(const MatchIterator &other) const noexcept { return m_idx == other.m_idx; }
	/// @brief Compare two MatchIterators
	auto operator!=(const MatchIterator &other) const noexcept { return !operator==(other); }

	/**
	 * @brief Get current index of the match
	 * @return Index of the match.
	 */
	auto index() const noexcept { return m_idx; }
private:
	friend struct Matches;
	friend class PCRE2;
	static std::string_view matchByIdx(size_t *ovector, std::string_view subject,
					   unsigned index) {
		const auto start = ovector[2 * index];
		const auto len = ovector[2 * index + 1] - start;
		return subject.substr(start, len);
	}

	explicit MatchIterator(unsigned int idx, size_t *ovector = nullptr,
			       std::string_view subject = "")
		: m_idx(idx), m_ovector(ovector), m_subject(std::move(subject)) {}

	unsigned m_idx;
	size_t *m_ovector;
	std::string_view m_subject;
};

/**
 * @brief Pseudo-vector of matches
 */
struct Matches {
	Matches() = delete;

	/// @brief Get first match (the begin iterator)
	auto begin() const noexcept { return MatchIterator(0, m_ovector, m_subject); }
	/// @brief Get past last match (the end iterator)
	auto end() const noexcept { return MatchIterator(m_matches); }

	/**
	 * @brief Get n-th match
	 * @param idx Index of the requested match
	 * @return \p idx-th match.
	 */
	auto operator[](std::size_t idx) const noexcept {
		return MatchIterator::matchByIdx(m_ovector, m_subject, idx);
	}

private:
	friend class PCRE2;

	explicit Matches(std::string_view subject, size_t *ovector, unsigned matches)
		: m_matches(matches), m_ovector(ovector), m_subject(std::move(subject)) {}

	unsigned m_matches;
	size_t *m_ovector;
	std::string_view m_subject;
};

/**
 * @brief Perl-compatible regex
 */
class PCRE2 {
public:
	/// @brief Constructs an empty PCRE2
	PCRE2() noexcept : m_code(nullptr), m_matchData(nullptr) {}
	~PCRE2() noexcept { free(); }

	PCRE2(const PCRE2 &) = delete;
	PCRE2 &operator=(const PCRE2 &) = delete;

	/// @brief Move constructor
	PCRE2(PCRE2 &&other) noexcept : m_lastError(std::move(other.m_lastError)),
			m_code(other.m_code), m_matchData(other.m_matchData) {
		other.m_code = nullptr;
		other.m_matchData = nullptr;
	}
	/// @brief Move assignment
	PCRE2 &operator=(PCRE2 &&other) noexcept {
		if (this != &other) {
			free();
			m_lastError = std::move(other.m_lastError);
			std::swap(m_code, other.m_code);
			std::swap(m_matchData, other.m_matchData);
		}
		return *this;
	}

	/**
	 * @brief Compile PCRE2 \p regex with passed \p options
	 * @param regex Regex to compile
	 * @param options Options to use (like PCRE2_CASELESS)
	 * @return true on success
	 */
	bool compile(std::string_view regex, uint32_t options = 0) noexcept {
		free();

		int err;
		PCRE2_SIZE lastOff;
		m_code = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(regex.data()), regex.length(),
				       options, &err, &lastOff, nullptr);
		if (!m_code) {
			m_lastError.reset().setError(errToStr(err));
			m_lastError.set<0>(err);
			m_lastError.set<1>(lastOff);
			return false;
		}

		m_matchData = pcre2_match_data_create_from_pattern(m_code, nullptr);
		if (!m_matchData) {
			m_lastError.reset().setError("failed to allocate match data");
			m_lastError.set<0>(PCRE2_ERROR_NOMEMORY);
			return false;
		}

		return true;
	}

	/**
	 * @brief PCRE2 against \p subject
	 * @param subject Text to match the regex to
	 * @return negative on error, 1 for no match, 2 for one group matched, ...
	 */
	int match(std::string_view subject) noexcept {
		return pcre2_match(m_code, reinterpret_cast<PCRE2_SPTR>(subject.data()),
				   subject.length(), 0, 0, m_matchData, nullptr);
	}

	/**
	 * @brief Returns offset (into the subject string) vector for all matches
	 * @return An array of size_t values.
	 *
	 * Preferrably, use matches() or matchByIdx() wrappers.
	 */
	auto ovector() const { return pcre2_get_ovector_pointer(m_matchData); }

	/**
	 * @brief Converts PCRE2 error code \p err to string
	 * @param err PCRE2 error code
	 * @return Error string.
	 */
	static std::string errToStr(int err) {
		std::string s(256, 0);
		auto len = pcre2_get_error_message(err, reinterpret_cast<PCRE2_UCHAR *>(s.data()),
						   s.length());
		if (len < 0)
			return {};
		s.resize(len);
		return s;
	}

	/**
	 * @brief Returns Matches (a pseudo-vector of matches) in \p subject
	 * @param subject Subject matched
	 * @param matches Number of matches -- the return value of match()
	 * @return Pseudo-vector of matches.
	 */
	auto matches(std::string_view subject, unsigned matches) const noexcept {
		return Matches(subject, ovector(), matches);
	}

	/**
	 * @brief Returns one match -- a substring of \p subject
	 * @param subject Subject matched
	 * @param index Index of the requested match
	 * @return Matched string.
	 */
	auto matchByIdx(std::string_view subject, unsigned index) const noexcept {
		return MatchIterator::matchByIdx(ovector(), subject, index);
	}

	/// @brief Return the last error number
	auto lastErrno() const noexcept { return m_lastError.get<0>(); }
	/// @brief Return the last error string if some
	auto lastError() const noexcept { return m_lastError.lastError(); }
	/// @brief Get offset of last error (to the regex string)
	auto lastOffset() const noexcept { return m_lastError.get<1>(); }

	/// @brief Test whether PCRE2 is valid
	bool valid() const noexcept { return m_code; }
	/// @brief bool wrapper around valid()
	operator bool() const noexcept { return valid(); }
	/// @brief ! wrapper around valid()
	bool operator!() const noexcept { return !valid(); }
private:
	void free() {
		pcre2_match_data_free(m_matchData);
		m_matchData = nullptr;
		pcre2_code_free(m_code);
		m_code = nullptr;
	}
	SlHelpers::LastErrorStr<int, PCRE2_SIZE> m_lastError;
	pcre2_code *m_code;
	pcre2_match_data *m_matchData;
};

}
