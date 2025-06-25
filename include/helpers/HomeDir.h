// SPDX-License-Identifier: GPL-2.0-only

#ifndef HOMEDIR_H
#define HOMEDIR_H

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>

namespace SlHelpers {

class HomeDir {
public:
	HomeDir() = delete;

	static std::filesystem::path get() {
		std::filesystem::path dir;

		if (auto homeDir = std::getenv("HOME")) {
			dir = homeDir;
			if (std::filesystem::exists(dir))
				return dir;
		}

		dir = ::getpwuid(::getuid())->pw_dir;
		if (std::filesystem::exists(dir))
			return dir;

		return "";
	}
};

}

#endif
