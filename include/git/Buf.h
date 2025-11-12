// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLGIT_BUF_H
#define SLGIT_BUF_H

#include <string>
#include <string_view>

#include <git2.h>

namespace SlGit {

class Buf {
public:
	Buf() : m_buf(GIT_BUF_INIT) {}
	~Buf() { git_buf_dispose(&m_buf); }

	Buf(const Buf &other) = delete;
	Buf operator=(const Buf &other) = delete;
	Buf(Buf &&other) noexcept {
		m_buf = other.m_buf;
		other.m_buf = GIT_BUF_INIT;
	}
	Buf &operator=(Buf &&other) noexcept {
		if (this != &other) {
			git_buf_dispose(&m_buf);
			m_buf = other.m_buf;
			other.m_buf = GIT_BUF_INIT;
		}
		return *this;
	}

	std::string_view sv() const noexcept { return std::string_view(m_buf.ptr, m_buf.size); }
	std::string str() const noexcept { return std::string(m_buf.ptr, m_buf.size); }

	git_buf &buf() { return m_buf; }
	const git_buf &buf() const { return m_buf; }
	operator const git_buf *() const { return &m_buf; }
private:
	git_buf m_buf;
};

}

#endif
