// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "helpers/Ratelimit.h"

int main()
{
	SlHelpers::Ratelimit r(std::chrono::milliseconds(100));
	auto counter = 0U;

	for (auto i = 0U; i < 99U; ++i) {
		if (r.limit())
			counter++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	std::cerr << "counter=" << counter << '\n';
	assert(counter <= 10);

	return 0;
}
