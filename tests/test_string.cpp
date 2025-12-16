// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <sstream>

#include "helpers/String.h"

using namespace SlHelpers;

namespace {

void testSplit()
{
	static const constexpr std::string_view toSplit("  \t first second    third\t\tfourth  # ignore ");

	{
		const auto split = String::split(std::string(toSplit), " \t");
		assert(split.size() == 6);
		assert(split[0] == "first");
		assert(split[1] == "second");
		assert(split[2] == "third");
		assert(split[3] == "fourth");
		assert(split[4] == "#");
		assert(split[5] == "ignore");
	}
	{
		const auto split = String::split(std::string(toSplit), " ", '#');
		assert(split.size() == 4);
		assert(split[0] == "\t");
		assert(split[1] == "first");
		assert(split[2] == "second");
		assert(split[3] == "third\t\tfourth");
	}
	{
		const auto split = String::splitSV(toSplit, " \t");
		assert(split.size() == 6);
		assert(split[0] == "first");
		assert(split[1] == "second");
		assert(split[2] == "third");
		assert(split[3] == "fourth");
		assert(split[4] == "#");
		assert(split[5] == "ignore");
	}
	{
		const auto split = String::splitSV(toSplit, " ", '#');
		assert(split.size() == 4);
		assert(split[0] == "\t");
		assert(split[1] == "first");
		assert(split[2] == "second");
		assert(split[3] == "third\t\tfourth");
	}
}

void testIsHex()
{
	assert(String::isHex(""));
	assert(String::isHex("01234567890abcdefABCDEF"));
	assert(!String::isHex("01234567890abcdefABCDEFG"));
	assert(!String::isHex("x01234567890"));
}

void testTrim()
{
	assert(String::trim(std::string("")) == "");
	assert(String::trim(std::string_view("")) == "");
	assert(String::trim(std::string_view(" ")) == "");
	assert(String::trim(std::string_view("\n\n \t")) == "");
	assert(String::trim(std::string_view("\n\nx \t")) == "x");
	assert(String::trim(std::string_view("x \t")) == "x");
	assert(String::trim(std::string_view("\n\nx")) == "x");
	assert(String::trim(std::string_view("x")) == "x");
}

void testIFind()
{
	assert(String::iFind("", "") == 0);
	assert(String::iFind("abc", "") == 0);
	assert(String::iFind("abc", "b") == 1);
	assert(String::iFind("abc", "B") == 1);
	assert(String::iFind("abc", "c") == 2);
	assert(String::iFind("abc", "abc") == 0);
	assert(String::iFind("abc", "ABC") == 0);
	assert(String::iFind("abc", "abcd") == std::string_view::npos);
	assert(String::iFind("abc", "x") == std::string_view::npos);

	assert(String::iFind(std::string_view("abc"), "b") == 1);
	assert(String::iFind(std::string_view("abc"), std::string("b")) == 1);
	assert(String::iFind(std::string_view("abc"), std::string_view("b")) == 1);
}

void testJoin()
{
	{
		std::ostringstream ss;
		String::join(ss, std::vector<std::string_view>());
		assert(ss.str() == "");
	}
	{
		std::ostringstream ss;
		String::join(ss, std::vector<std::string_view>({"a"}));
		assert(ss.str() == "a");
	}
	{
		std::ostringstream ss;
		String::join(ss, std::vector<std::string_view>({"a", "b", "c"}));
		assert(ss.str() == "a, b, c");
	}
	{
		std::ostringstream ss;
		String::join(ss, std::vector<std::string_view>({"a", "b", "c"}), ",", "x");
		assert(ss.str() == "xax,xbx,xcx");
	}
}

void testGetLine()
{
	{
		static const constexpr std::string_view lines[] = {
			"one", "two", "three", "four"
		};
		std::ostringstream ss;
		for (const auto &l: lines)
			ss << l << '\n';

		GetLine gl(ss.view());
		auto i = 0U;
		while (auto line = gl.get())
			assert(lines[i++] == line);
		assert(i == 4);
	}
	{
		assert(!GetLine("").get());

	}
	{
		GetLine gl("one\ntwo");
		assert(gl.get() == "one");
		assert(gl.get() == "two");
		assert(!gl.get());

	}
}

} // namespace

int main()
{
	testSplit();
	testIsHex();
	testTrim();
	testIFind();
	testJoin();
	testGetLine();

	return 0;
}

