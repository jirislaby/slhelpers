// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "cves/CVE.h"

using namespace SlCVEs;

namespace {

void testCVEHashMap()
{
	assert(!CVE::getCVENumber("x"));
	assert(CVE::getCVENumber("CVE-2025-1") == "CVE-2025-1");
	assert(CVE::getCVENumber("CVE-2025-12345678") == "CVE-2025-12345678");
	assert(CVE::getCVENumber("CVE-2025-12345678.sha1") == "CVE-2025-12345678");
}

} // namespace

int main()
{
	testCVEHashMap();

	return 0;
}

