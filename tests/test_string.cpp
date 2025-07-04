#include <cassert>

#include "helpers/String.h"

using namespace SlHelpers;

int main()
{
	assert(String::startsWith("abcde", "abcde"));
	assert(String::startsWith("abcde", "abc"));

	assert(!String::startsWith("a", "aa"));
	assert(!String::startsWith("abcde", "abcc"));
	assert(!String::startsWith("abcde", "bc"));

	assert(String::endsWith("abcde", "cde"));
	assert(String::endsWith("abcde", "abcde"));

	assert(!String::endsWith("a", "aa"));
	assert(!String::endsWith("abcde", "cdef"));
	assert(!String::endsWith("abcde", "cd"));

	return 0;
}

