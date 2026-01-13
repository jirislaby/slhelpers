// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string>

#include <git2.h>

namespace SlGit {

/**
 * @brief Helpers for SlGit
 */
class Helpers {
public:
	Helpers() = delete;

	/// @brief Convert OID \p id to string
	static std::string oidToStr(const git_oid &id) {
		std::string s(GIT_OID_MAX_HEXSIZE, '\0');
		git_oid_tostr(s.data(), s.size() + 1, &id);
		return s;
	}

	/// @brief Convert string \p sv to git_oid
	static std::optional<git_oid> strToOid(std::string_view sv) {
		git_oid oid;
		if (git_oid_fromstrn(&oid, sv.data(), sv.size()))
			return std::nullopt;
		return oid;
	}
};

}
