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
	 * When the destructor is called, it is cd-ed back to the original directory (the current
	 * directory before the call to constructor).
	 */
	PushD(const std::filesystem::path &dir, std::error_code &ec) {
		origDir = std::filesystem::current_path(ec);
		if (!ec)
			std::filesystem::current_path(dir, ec);
	}

	/**
	 * @brief Change to directory \p dir and throw an exception on error
	 * @param dir Directory to cd to
	 *
	 * When the destructor is called, it is cd-ed back to the original directory (the current
	 * directory before the call to constructor).
	 */
	PushD(const std::filesystem::path &dir) {
		origDir = std::filesystem::current_path();
		std::filesystem::current_path(dir);
	}
	~PushD() { std::filesystem::current_path(origDir); }
private:
	std::filesystem::path origDir;
};

}
