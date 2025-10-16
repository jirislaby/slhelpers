// SPDX-License-Identifier: GPL-2.0-only

#ifndef HOMEDIR_H
#define HOMEDIR_H

#include <filesystem>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace SlHelpers {

class HomeDir {
public:
	HomeDir() = delete;

	/**
	 * @brief Obtains home directory
	 * @return $HOME or /etc/passwd home entry
	 */
	static std::filesystem::path get() {
		std::filesystem::path dir;

		if (const auto homeDir = std::getenv("HOME")) {
			dir = homeDir;
			if (std::filesystem::exists(dir))
				return dir;
		}

		dir = ::getpwuid(::getuid())->pw_dir;
		if (std::filesystem::exists(dir))
			return dir;

		return {};
	}

	/**
	 * @brief Obtains directory for caching
	 * @return $XDG_CACHE_HOME or $HOME/.cache
	 */
	static std::filesystem::path getCacheDir()
	{
		if (const auto xdgCacheDir = std::getenv("XDG_CACHE_HOME"))
			return xdgCacheDir;

		const auto home_dir = get();
		if (home_dir.empty())
			return {};

		return std::filesystem::path(home_dir) / ".cache";
	}

	/**
	 * @brief Creates (if not existing) and returns getCacheDir() / subdir
	 * @param subdir Subdirectory to append to cache dir
	 * @return Created getCacheDir() / subdir
	 */
	static std::filesystem::path createCacheDir(const std::filesystem::path &subdir)
	{
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

#endif
