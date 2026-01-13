// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <string_view>

#include <git2.h>

namespace SlGit {

/**
 * @brief Buf is a representation of a git buffer
 */
class Buf {
public:
	Buf() : m_buf(GIT_BUF_INIT) {}
	~Buf() { git_buf_dispose(&m_buf); }

	Buf(const Buf &other) = delete;
	Buf operator=(const Buf &other) = delete;

	/// @brief Move constructor
	Buf(Buf &&other) noexcept : m_buf(other.m_buf) { other.m_buf = GIT_BUF_INIT; }
	/// @brief Move assignment
	Buf &operator=(Buf &&other) noexcept {
		if (this != &other) {
			git_buf_dispose(&m_buf);
			m_buf = other.m_buf;
			other.m_buf = GIT_BUF_INIT;
		}
		return *this;
	}

	/// @brief Get this Buf as a string_view
	std::string_view sv() const noexcept { return std::string_view(m_buf.ptr, m_buf.size); }
	/// @brief Get this Buf as a string
	std::string str() const noexcept { return std::string(m_buf.ptr, m_buf.size); }

	/// @brief Get the stored libgit2's git_buf
	git_buf &buf() { return m_buf; }
	/// @brief Get the stored libgit2's git_buf
	const git_buf &buf() const { return m_buf; }
	/// @brief Alias for buf() -- implicit conversion
	operator const git_buf *() const { return &m_buf; }
private:
	git_buf m_buf;
};

}
