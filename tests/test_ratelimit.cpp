// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "helpers/Ratelimit.h"

int main()
{
	constexpr const std::chrono::milliseconds rlDur {100};
	SlHelpers::Ratelimit r(rlDur);
	auto counter = 0U;

	const auto start = std::chrono::steady_clock::now();
	for (auto i = 1U; i <= 100U; ++i) {
		if (r.limit())
			counter++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	const auto dur = std::chrono::steady_clock::now() - start;
	const auto maxCount = dur / rlDur + 1;

	std::cerr << "took=" <<
		     std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur).count() <<
		     " ms checking " << counter << " <= " << maxCount << '\n';
	assert(counter <= maxCount);

	return 0;
}
