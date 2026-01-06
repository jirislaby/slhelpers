// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>

#include <git2.h>

namespace SlGit {

class Helpers {
public:
	static std::string oidToStr(const git_oid &id) {
		char buf[GIT_OID_MAX_HEXSIZE + 1];
		git_oid_tostr(buf, sizeof(buf), &id);
		return buf;
	}

};

}
