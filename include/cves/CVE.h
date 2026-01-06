// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string_view>

namespace SlCVEs {

/**
 * @brief Helper class for CVE numbers
 */
class CVE {
public:
	/**
	 * @brief Try to parse \p sv as a CVE number and return it
	 * @param sv String to parse
	 * @return CVE number or nullopt on failure to parse
	 */
	static std::optional<std::string_view> getCVENumber(std::string_view sv) noexcept;
};

}
