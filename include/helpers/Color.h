// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_COLOR_H
#define SLHELPERS_COLOR_H

#include <iostream>
#include <unistd.h>

namespace SlHelpers {

class Color {
public:
	enum C {
		BLACK = 30,
		RED = 31,
		GREEN = 32,
		YELLOW = 33,
		BLUE = 34,
		MAGENTA = 35,
		CYAN = 36,
		WHITE = 37,
		COL256 = 38,
		DEFAULT = 39,
	};
	enum Ctrl {
		NoNL,
		NL,
	};

	Color(const enum C &c) : Color(std::cout, c) {}
	Color(std::ostream &os, const enum C &c) : m_NL(true), m_os(os) {
		if (doColor(os))
			m_os << seqBegin << toUnsigned(c) << 'm';
	}

	Color(unsigned char r, unsigned char g, unsigned char b) : Color(std::cout, r, g, b) {}
	Color(std::ostream &os, unsigned char r, unsigned char g, unsigned char b) :
		m_NL(true), m_os(os) {
		if (!doColor(os))
			return;
		m_os << seqBegin << toUnsigned(COL256) << ";2;" <<
			toUnsigned(r) << ';' <<
			toUnsigned(g) << ';' <<
			toUnsigned(b) << 'm';
	}

	~Color() {
		if (doColor(m_os))
			m_os << seqEnd;
		if (m_NL)
			m_os << '\n';
	}

	static void forceColor(bool force) { m_forceColor = force; }
	static void forceColorValue(bool value) { m_forceColorValue = value; }

	std::ostream &os() { return m_os; }

	void ctrl(const Color::Ctrl &ctrl) {
		switch (ctrl) {
		case NL:
		case NoNL:
			m_NL = ctrl == NL;
			break;
		}
	}

private:
	template<typename T>
	static unsigned toUnsigned(const T &val) { return val; }
	static unsigned outIndex(const std::ostream &os) {
		if (os.rdbuf() == std::cout.rdbuf())
			return 0U;
		if (os.rdbuf() == std::cerr.rdbuf())
			return 1U;
		return 2U;
	}
	static bool doColor(const std::ostream &os) {
		if (m_forceColor)
			return m_forceColorValue;
		auto idx = outIndex(os);
		if (m_doColor[idx] < 0)
			m_doColor[idx] = isatty(idx + 1);
		return m_doColor[idx];
	}

	inline static std::string seqBegin = "\033[01;";
	inline static std::string seqEnd = "\033[0m";
	inline static signed char m_doColor[] = { -1, -1, 1 };
	inline static bool m_forceColor = false;
	inline static bool m_forceColorValue = false;
	bool m_NL;
	std::ostream &m_os;
};

inline Color &&operator<<(Color &&os, const Color::Ctrl &ctrl)
{
	os.ctrl(ctrl);
	return std::move(os);
}

template<typename T>
Color &&operator<<(Color &&os, const T &x)
{
	os.os() << x;
	return std::move(os);
}

}

#endif
