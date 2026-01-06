// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <vector>

#include <git2.h>

namespace SlGit {

class StrArray {
public:
	StrArray(const std::vector<std::string> &vec) {
		for (const auto &entry : vec)
			strings.push_back(entry.c_str());
		m_array.strings = const_cast<char **>(strings.data());
		m_array.count = strings.size();
	}

	const git_strarray *array() const { return &m_array; }
	operator const git_strarray *() const { return &m_array; }
private:
	std::vector<const char *> strings;
	git_strarray m_array;
};

}
