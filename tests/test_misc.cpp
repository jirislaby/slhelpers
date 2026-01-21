// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/Misc.h"

#include "helpers.h"

using namespace SlHelpers;

namespace {

void testVersion()
{
	assert(Version::versionSum("1") == 1U << 16);
	assert(Version::versionSum("1.2") == ((1U << 16) | (2U << 8)));
	assert(Version::versionSum("1.100.150") == ((1U << 16) | (100U << 8) | 150));
}

void testCmpVersions()
{
	CmpVersions cmp;

	assert(cmp("1", "2"));
	assert(cmp("2", "3"));
	assert(cmp("1", "3"));
	assert(cmp("1", "1.1"));
	assert(cmp("1.1", "1.2"));
	assert(cmp("1.1", "1.1.1"));
	assert(cmp("1.1.1", "1.1.2"));
	assert(cmp("1.1-rc1", "1.1-rc2"));

	assert(!cmp("1", "1"));
	assert(!cmp("2", "1"));
	assert(!cmp("1.1-rc2", "1.1-rc1"));
}

void testHuman()
{
	assert(Unit::human(0) == "0.00 B");
	assert(Unit::human(10) == "10.00 B");
	assert(Unit::human((1 << 10) - 1) == "1023.00 B");

	assert(Unit::human(1 << 10) == "1.00 KiB");
	assert(Unit::human(10 << 10) == "10.00 KiB");
	assert(Unit::human((1 << 20) - (1 << 10)) == "1023.00 KiB");

	assert(Unit::human(10 << 20) == "10.00 MiB");

	if (sizeof(size_t) >= 8) {
		assert(Unit::human(10ULL << 30) == "10.00 GiB");
		assert(Unit::human(10ULL << 40) == "10.00 TiB");
		assert(Unit::human(10ULL << 50) == "10.00 PiB");
		assert(Unit::human(10ULL << 60) == "10.00 EiB");

		assert(Unit::human(10ULL << 60, 0) == "10 EiB");
		assert(Unit::human(10ULL << 60, 5) == "10.00000 EiB");
	}

	assert(Unit::human(static_cast<size_t>(10.5 * 1024)) == "10.50 KiB");
}

void testEnv()
{
	THelpers::RestoreEnv env(__func__ + std::to_string(rand()));

	::unsetenv(env.env().c_str());
	assert(Env::get(env.env()) == std::nullopt);

	{
		const std::string strValue{"some_string"};
		::setenv(env.env().c_str(), strValue.c_str(), true);
		assert(Env::get<decltype(strValue)>(env.env()) == strValue);
	}

	{
		const std::filesystem::path pathValue{"/some/path/to_file"};
		::setenv(env.env().c_str(), pathValue.c_str(), true);
		assert(Env::get<decltype(pathValue)>(env.env()) == pathValue);
	}
}

}

int main()
{
	testVersion();
	testCmpVersions();
	testHuman();
	testEnv();

	return 0;
}

