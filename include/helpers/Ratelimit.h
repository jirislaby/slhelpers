// SPDX-License-Identifier: GPL-2.0-only

#ifndef RATELIMIT_H
#define RATELIMIT_H

#include <chrono>

namespace SlHelpers {

class Ratelimit {
public:
	using Clock = std::chrono::steady_clock;

	Ratelimit() = delete;
	Ratelimit(const std::chrono::milliseconds &dur) : dur(dur), last(Clock::time_point{}) { }

	void reset() { last = Clock::time_point{}; }

	bool limit() {
		const auto now = std::chrono::steady_clock::now();
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
