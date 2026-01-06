// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <chrono>

namespace SlHelpers {

/**
 * @brief Rate-limit some actions
 */
class Ratelimit {
public:
	/// @brief Clock used for measuring
	using Clock = std::chrono::steady_clock;

	Ratelimit() = delete;

	/**
	 * @brief Construct new Ratelimit, allowing action to be once per \p dur
	 * @param dur Duration between actions
	 */
	Ratelimit(const std::chrono::milliseconds &dur) : dur(dur), last(Clock::time_point{}) { }

	/// @brief Start counting from now
	void reset() { last = Clock::time_point{}; }

	/**
	 * @brief Limit actions to one per constructor's dur
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
