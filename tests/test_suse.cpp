// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/SUSE.h"

using namespace SlHelpers;

namespace {

void testSUSE()
{
	assert(SUSE::isSUSEAddress("franta@suse.com"));
	assert(SUSE::isSUSEAddress("franta@suse.cz"));
	assert(SUSE::isSUSEAddress("franta@suse.de"));
	assert(!SUSE::isSUSEAddress("franta@domain.com"));
}

}

int main()
{
	testSUSE();

	return 0;
}

