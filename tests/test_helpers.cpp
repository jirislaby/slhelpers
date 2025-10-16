// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/HomeDir.h"
#include "helpers/Process.h"

using namespace SlHelpers;

namespace {

void restore_env(const std::string &env, const char *orig)
{
	if (orig)
		setenv(env.c_str(), orig, true);
	else
		unsetenv(env.c_str());
}

void testHomeDir()
{
	auto orig_home = std::getenv("HOME");
	if (orig_home)
		assert(HomeDir::get() == orig_home);

	setenv("HOME", "/tmp", true);
	assert(HomeDir::get() == "/tmp");
	restore_env("HOME", orig_home);
}

void testProcess(const std::filesystem::path &crash)
{
	Process p;

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

	assert(!p.run(crash));
	assert(p.signalled());
}

}

int main(int argc, char **argv)
{
	assert(argc > 1);
	testHomeDir();
	testProcess(argv[1]);

	return 0;
}

