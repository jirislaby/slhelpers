// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/String.h"

using namespace SlHelpers;

static void testStartsWith()
{
	assert(String::startsWith("abcde", "abcde"));
	assert(String::startsWith("abcde", "abc"));

	assert(!String::startsWith("a", "aa"));
	assert(!String::startsWith("abcde", "abcc"));
	assert(!String::startsWith("abcde", "bc"));
}

static void testEndsWith()
{
	assert(String::endsWith("abcde", "cde"));
	assert(String::endsWith("abcde", "abcde"));

	assert(!String::endsWith("a", "aa"));
	assert(!String::endsWith("abcde", "cdef"));
	assert(!String::endsWith("abcde", "cd"));
}

static void testSplit()
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

int main()
{
	testStartsWith();
	testEndsWith();
	testSplit();

	return 0;
}

