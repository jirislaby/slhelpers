// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>

#include "kerncvs/Pattern.h"

using namespace SlKernCVS;

constexpr unsigned int Pattern::pattern_weight(std::string_view pattern)
{
	unsigned ret = 1;
	bool seen = false;
	// "fs/udf/" and "fs/*" would have the same weight otherwise
	if (pattern.ends_with('*'))
		pattern.remove_suffix(1);
	for (const char c: pattern) {
		switch (c) {
		case '/':
			seen = true;
			break;
		case ' ':
		case '\n':
		case '\t':
		case '\r':
		case '\\':
			break;
		default:
			if (seen) {
				++ret;
				seen = false;
			}
		}
	}
	return ret;
}

std::optional<Pattern> Pattern::create(std::string pattern)
{
	if (!pattern.empty() && pattern.back() == '/' &&
			pattern.find_first_of('*') != std::string::npos)
		pattern.push_back('*');

	auto weight = pattern_weight(pattern);
	auto pathspec = SlGit::PathSpec::create({ std::move(pattern) });
	if (!pathspec) {
		std::cerr << git_error_last()->message << '\n';
		return std::nullopt;
	}

	return Pattern(std::move(*pathspec), weight);
}
