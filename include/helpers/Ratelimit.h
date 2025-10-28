// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_RATELIMIT_H
#define SLHELPERS_RATELIMIT_H

#include <chrono>

namespace SlHelpers {

class Ratelimit {
public:
	using Clock = std::chrono::steady_clock;

	Ratelimit() = delete;
	Ratelimit(const std::chrono::milliseconds &dur) : dur(dur), last(Clock::time_point{}) { }

	/**
	 * @brief start counting from now
	 */
	void reset() { last = Clock::time_point{}; }

	/**
	 * @brief limit actions to one per constructor's dur
	 * @return true if the ratelimited action should be performed
	 */
	bool limit() {
		const auto now = Clock::now();
		if (last + dur < now) {
			last = now;
			return true;
		}

		return false;
	}
private:
	const std::chrono::milliseconds dur;
	Clock::time_point last;
};

}

#endif
