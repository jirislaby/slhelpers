// SPDX-License-Identifier: GPL-2.0-only

#ifndef SSH_H
#define SSH_H

#include <string>
#include <vector>

#include "HomeDir.h"

namespace SlSSH {

class Keys {
public:
	Keys() = delete;
	static std::vector<std::string> get(const std::string &host) {
		(void)host;
		auto home = SlHelpers::HomeDir::get();
		return { home };
	}
};
}

#endif
