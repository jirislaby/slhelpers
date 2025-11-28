// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <sstream>

#include "helpers/String.h"

using namespace SlHelpers;

namespace {

void testStartsWith()
{
	assert(String::startsWith("abcde", "abcde"));
	assert(String::startsWith("abcde", "abc"));

	assert(!String::startsWith("a", "aa"));
	assert(!String::startsWith("abcde", "abcc"));
	assert(!String::startsWith("abcde", "bc"));
}

void testEndsWith()
{
	assert(String::endsWith("abcde", "cde"));
	assert(String::endsWith("abcde", "abcde"));

	assert(!String::endsWith("a", "aa"));
	assert(!String::endsWith("abcde", "cdef"));
	assert(!String::endsWith("abcde", "cd"));
}

void testSplit()
{
	std::string toSplit {"first second    third\t\tfourth  " };

	auto split1 = String::split(toSplit, " \t");
	assert(split1.size() == 4);
	assert(split1[0] == "first");
	assert(split1[1] == "second");
	assert(split1[2] == "third");
	assert(split1[3] == "fourth");

	auto split2 = String::split(toSplit, " ");
	assert(split2.size() == 3);
	assert(split2[0] == "first");
	assert(split2[1] == "second");
	assert(split2[2] == "third\t\tfourth");
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

} // namespace

int main()
{
	testStartsWith();
	testEndsWith();
	testSplit();
	testIsHex();
	testTrim();
	testIFind();
	testJoin();

	return 0;
}

