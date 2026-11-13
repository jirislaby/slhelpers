// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "Misc.h"

namespace SlHelpers {

/**
 * @brief Helpers for $HOME directory
 */
class HomeDir {
public:
	HomeDir() = delete;

	/**
	 * @brief Obtains home directory
	 * @return $HOME or /etc/passwd home entry.
	 */
	static std::filesystem::path get() noexcept {
		if (auto homeDir = Env::get<std::filesystem::path>("HOME"))
			if (std::filesystem::exists(*homeDir))
				return std::move(*homeDir);

		if (const auto pw = ::getpwuid(::getuid())) {
			std::filesystem::path dir = pw->pw_dir;
			if (std::filesystem::exists(dir))
				return dir;
		}

		return {};
	}

	/**
	 * @brief Obtains directory for caching
	 * @return $XDG_CACHE_HOME or $HOME/.cache.
	 */
	static std::filesystem::path getCacheDir() noexcept {
		// XDG spec: empty or relative values are to be ignored
		if (auto xdgCacheDir = Env::get<std::filesystem::path>("XDG_CACHE_HOME"))
			if (xdgCacheDir->is_absolute())
				return std::move(*xdgCacheDir);

		const auto homeDir = get();
		if (homeDir.empty())
			return {};

		return homeDir / ".cache";
	}

	/**
	 * @brief Creates (if not existing) and returns getCacheDir() / \p subdir
	 * @param subdir Subdirectory to append to cache dir
	 * @return Created getCacheDir() / \p subdir.
	 */
	static std::filesystem::path createCacheDir(const std::filesystem::path &subdir) noexcept {
		auto cache = getCacheDir();
		if (cache.empty())
			return {};
		cache /= subdir;

		std::error_code ec;
		std::filesystem::create_directories(cache, ec);
		if (ec)
			return {};

		return cache;
	}
};

}
