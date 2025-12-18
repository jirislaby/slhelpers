// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <pcre2.h>
#include <string_view>

#include "../helpers/LastError.h"

namespace SlPCRE2 {

struct MatchIterator {
	MatchIterator() = delete;

	auto operator*() const noexcept {
		return matchByIdx(m_ovector, m_subject, m_idx);
	}

	MatchIterator &operator++() noexcept { ++m_idx; return *this; }
	MatchIterator &operator--() noexcept { --m_idx; return *this; }
	MatchIterator operator++(int) noexcept {
		auto old = *this;
		++m_idx;
		return old;
	}
	MatchIterator operator--(int) noexcept {
		auto old = *this;
		--m_idx;
		return old;
	}
	auto operator==(const MatchIterator &other) const noexcept { return m_idx == other.m_idx; }
	auto operator!=(const MatchIterator &other) const noexcept { return !operator==(other); }

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

struct Matches {
	Matches() = delete;

	auto begin() const noexcept { return MatchIterator(0, m_ovector, m_subject); }
	auto end() const noexcept { return MatchIterator(m_matches); }

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

class PCRE2 {
public:
	PCRE2() noexcept : m_lastErrno(0), m_lastOffset(0), m_code(nullptr), m_matchData(nullptr) {}
	~PCRE2() noexcept { free(); }

	PCRE2(const PCRE2 &) = delete;
	PCRE2 &operator=(const PCRE2 &) = delete;

	PCRE2(PCRE2 &&other) noexcept : m_lastErrno(other.m_lastErrno),
			m_lastOffset(other.m_lastOffset), m_code(other.m_code),
			m_matchData(other.m_matchData) {
		other.m_code = nullptr;
		other.m_matchData = nullptr;
	}
	PCRE2 &operator=(PCRE2 &&other) noexcept {
		if (this != &other) {
			free();
			m_lastErrno = other.m_lastErrno;
			m_lastError = std::move(other.m_lastError);
			m_lastOffset = other.m_lastOffset;
			std::swap(m_code, other.m_code);
			std::swap(m_matchData, other.m_matchData);
		}
		return *this;
	}

	bool compile(std::string_view regex, uint32_t options = 0) noexcept {
		free();

		int err;
		m_code = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(regex.data()), regex.length(),
				       options, &err, &m_lastOffset, nullptr);
		if (!m_code) {
			m_lastErrno = err;
			m_lastError.reset() << errToStr(err);
			return false;
		}

		m_matchData = pcre2_match_data_create_from_pattern(m_code, nullptr);
		if (!m_matchData) {
			m_lastErrno = PCRE2_ERROR_NOMEMORY;
			m_lastError.reset() << "failed to allocate match data";
			return false;
		}

		return true;
	}

	int match(std::string_view subject) noexcept {
		return pcre2_match(m_code, reinterpret_cast<PCRE2_SPTR>(subject.data()),
				   subject.length(), 0, 0, m_matchData, nullptr);
	}

	auto ovector() const { return pcre2_get_ovector_pointer(m_matchData); }

	static std::string errToStr(int err) {
		std::string s(256, 0);
		auto len = pcre2_get_error_message(err, reinterpret_cast<PCRE2_UCHAR *>(s.data()),
						   s.length());
		if (len < 0)
			return {};
		s.resize(len);
		return s;
	}

	auto matches(std::string_view subject, unsigned matches) const noexcept {
		return Matches(subject, ovector(), matches);
	}

	auto matchByIdx(std::string_view subject, unsigned index) const noexcept {
		return MatchIterator::matchByIdx(ovector(), subject, index);
	}

	auto lastErrno() const noexcept { return m_lastErrno; }
	auto lastError() const noexcept { return m_lastError.lastError(); }
	auto lastOffset() const noexcept { return m_lastOffset; }

	bool valid() const noexcept { return m_code; }
	operator bool() const noexcept { return valid(); }
	bool operator!() const noexcept { return !valid(); }
private:
	void free() {
		pcre2_match_data_free(m_matchData);
		m_matchData = nullptr;
		pcre2_code_free(m_code);
		m_code = nullptr;
	}
	int m_lastErrno;
	SlHelpers::LastError m_lastError;
	PCRE2_SIZE m_lastOffset;
	pcre2_code *m_code;
	pcre2_match_data *m_matchData;
};

}
