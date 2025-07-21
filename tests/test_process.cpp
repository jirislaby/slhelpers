// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/Process.h"

using namespace SlHelpers;

int main(int, char **argv)
{
	Process p;
	auto crash = argv[1];

	assert(!p.run("/usr/bin/true"));
	assert(!p.signalled());
	assert(!p.exitStatus());

	assert(!p.run("/usr/bin/false"));
	assert(!p.signalled());
	assert(p.exitStatus());

	assert(p.run("/does_not_exist/bin"));
	assert(p.lastErrorNo() == Process::SpawnError);

	std::string s;
	assert(!p.run("/usr/bin/echo", { "-e", "one", "two\\n\\tthree" }, &s));
	assert(!p.signalled());
	assert(!p.exitStatus());
	assert(s == "one two\n\tthree\n");

	assert(crash);
	assert(!p.run(crash));
	assert(p.signalled());

	return 0;
}

