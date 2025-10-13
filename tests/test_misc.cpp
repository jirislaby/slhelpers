// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/Misc.h"

using namespace SlHelpers;

static void testHuman()
{
	assert(Unit::human(0) == "0.00 B");
	assert(Unit::human(10) == "10.00 B");
	assert(Unit::human((1 << 10) - 1) == "1023.00 B");

	assert(Unit::human(1 << 10) == "1.00 KiB");
	assert(Unit::human(10 << 10) == "10.00 KiB");
	assert(Unit::human((1 << 20) - (1 << 10)) == "1023.00 KiB");

	assert(Unit::human(10 << 20) == "10.00 MiB");
	assert(Unit::human(10ULL << 30) == "10.00 GiB");
	assert(Unit::human(10ULL << 40) == "10.00 TiB");
	assert(Unit::human(10ULL << 50) == "10.00 PiB");
	assert(Unit::human(10ULL << 60) == "10.00 EiB");

	assert(Unit::human(10ULL << 60, 0) == "10 EiB");
	assert(Unit::human(10ULL << 60, 5) == "10.00000 EiB");

	assert(Unit::human(static_cast<size_t>(10.5 * 1024)) == "10.50 KiB");
}

int main()
{
	testHuman();

	return 0;
}

