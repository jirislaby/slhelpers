// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>

namespace SlHelpers {

/**
 * @brief Change to a directory while this object lives, then change back
 */
class PushD {
public:
	PushD() = delete;
	/**
	 * @brief Change to directory \p dir
	 * @param dir Directory to cd to
	 * @param ec Error code if something failed
	 *
	 * When the desctructor is called, it is cd-ed back to the original directory (the current
	 * directory before the call to constructor).
	 */
	PushD(const std::filesystem::path &dir, std::error_code &ec) {
		origDir = std::filesystem::current_path(ec);
		if (!ec)
			std::filesystem::current_path(dir, ec);
	}
	~PushD() { std::filesystem::current_path(origDir); }
private:
	std::filesystem::path origDir;
};
}
