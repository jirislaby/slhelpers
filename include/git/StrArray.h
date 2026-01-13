// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <vector>

#include <git2.h>

namespace SlGit {

/**
 * @brief StrArray is a representation of a git string array
 */
class StrArray {
public:
	/// @brief Constructs a StrArray from \p vec (must outlive StrArray)
	StrArray(const std::vector<std::string> &vec) {
		for (const auto &entry : vec)
			strings.push_back(entry.c_str());
		m_array.strings = const_cast<char **>(strings.data());
		m_array.count = strings.size();
	}

	/// @brief Get the stored pointer to libgit2's git_strarray
	const git_strarray *array() const { return &m_array; }
	/// @brief Alias for array() -- implicit conversion
	operator const git_strarray *() const { return &m_array; }
private:
	std::vector<const char *> strings;
	git_strarray m_array;
};

}
