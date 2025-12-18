// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <iostream>

#include "helpers/Color.h"
#include "pcre2/PCRE2.h"

using namespace SlPCRE2;

using Clr = SlHelpers::Color;

static_assert(!std::is_copy_constructible<PCRE2>());
static_assert(!std::is_copy_assignable<PCRE2>());

int main()
{
	PCRE2 regex2;
	assert(regex2.compile("dummy"));
	assert(regex2);

	{
		PCRE2 regex;

		assert(regex.compile("a(.*)(b)$"));
		assert(regex.valid());
		assert(regex);
		static constexpr std::string_view subject("axxxb");
		auto m = regex.match(subject);
		assert(m == 3);
		const auto matches = regex.matches(subject, m);
		auto I = matches.begin();
		assert(*I++ == subject);
		assert(*I++ == "xxx");
		--I;
		++I;
		assert(*I++ == "b");
		assert(I == matches.end());
		assert(matches[2] == "b");
		assert(regex.matchByIdx(subject, 2) == "b");

		regex2 = std::move(regex);
	}

	{
		auto regex(std::move(regex2));

		assert(regex.match("axxxB") == PCRE2_ERROR_NOMATCH);

		static constexpr std::string_view regexStr(".*.*$*X");
		assert(!regex.compile(regexStr));
		assert(!regex.valid());
		assert(!regex);
		assert(regex.lastErrno() == PCRE2_ERROR_QUANTIFIER_INVALID);
		Clr(std::cerr, Clr::GREEN) << "EXPECTED error: " << regex.lastError();
		assert(regex.lastError() == "quantifier does not follow a repeatable item");
		Clr(std::cerr, Clr::GREEN) << "for '" << regexStr << "' error starts at " <<
					      regex.lastOffset() << ": " <<
					      regexStr.substr(0, regex.lastOffset()) << " <-- HERE";

		// pre-10.47 yield 5 which seems to be more correct
		assert(regex.lastOffset() == 5 || regex.lastOffset() == 6);
	}

	{
		PCRE2 regex;
		assert(regex.compile("a(.*)(b)$", PCRE2_CASELESS));
		assert(regex.match("axxxB") > 0);
	}

	return 0;
}
