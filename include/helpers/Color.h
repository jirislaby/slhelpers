// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <iostream>
#include <unistd.h>

namespace SlHelpers {
/**
 * @brief Colorized output
 *
 * Use like:
 * @code
 * using Clr = SlHelpers::Color;
 * Clr(std::cerr, Clr::RED) << "error";
 * @endcode
 */
class Color {
public:
	/// @brief Predefined colors
	enum C : unsigned {
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

	/// @brief Controls for Color
	enum Ctrl {
		NoNL,
		NL,
	};

	/**
	 * @brief New Color stream (stdout) using color \p c
	 * @param c A color from C
	 */
	explicit Color(enum C c = DEFAULT) noexcept : Color(std::cout, c) {}

	/**
	 * @brief New Color stream (into \p os) using color \p c
	 * @param os Output stream where to output
	 * @param c A color from C
	 */
	explicit Color(std::ostream &os, enum C c = DEFAULT) noexcept : m_NL(true), m_os(os) {
		if (doColor(os))
			m_os << seqBegin << c << 'm';
	}

	/**
	 * @brief New Color stream (stdout) using an RGB color
	 * @param r Red
	 * @param g Green
	 * @param b Blue
	 */
	Color(unsigned char r, unsigned char g, unsigned char b) noexcept
		: Color(std::cout, r, g, b) {}

	/**
	 * @brief New Color stream (into \p os) using an RGB color
	 * @param os Output stream where to output
	 * @param r Red
	 * @param g Green
	 * @param b Blue
	 */
	Color(std::ostream &os, unsigned char r, unsigned char g, unsigned char b) noexcept
		: m_NL(true), m_os(os) {
		if (!doColor(os))
			return;
		m_os << seqBegin << COL256 << ";2;" <<
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

	/**
	 * @brief Force color value (see forceColorValue())
	 * @param force If set, use of colors is bound to color value set by forceColorValue()
	 */
	static void forceColor(bool force) { m_forceColor = force; }
	/**
	 * @brief If forceColor(true) was called, use of colors is bound to \p value
	 * @param value If colors should be used
	 */
	static void forceColorValue(bool value) { m_forceColorValue = value; }

	/**
	 * @brief Return current output stream
	 * @return Output stream.
	 */
	std::ostream &os() { return m_os; }

	/**
	 * @brief Controls the stream behavior
	 * @param ctrl Control character (like NoNL to not automatically emit a newline at the end)
	 */
	void ctrl(Ctrl ctrl) {
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

	friend void testColor();
	inline static std::string_view seqBegin = "\033[01;";
	inline static std::string_view seqEnd = "\033[0m";
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
