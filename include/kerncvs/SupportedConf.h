// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace SlKernCVS {

/**
 * @brief Parses supported.conf and holds/retrieves the information
 */
class SupportedConf {
public:
	/// @brief Level of support for a module
	enum SupportState {
		NonPresent = -3,
		Unsupported = -2,
		UnsupportedOptional = -1,
		Unspecified = 0,
		Supported = 1,
		BaseSupported = 2,
		ExternallySupported = 3,
		KMPSupported = 4,
	};

	SupportedConf() = delete;

	/**
	 * @brief Parse \p conf and store
	 * @param conf supported.conf content
	 */
	SupportedConf(std::string_view conf);

	/**
	 * @brief Find supported state of \p module
	 * @param module Module to find supported state of
	 * @return One of SupportState -- NonPresent if not found.
	 */
	SupportState supportState(const std::string &module) const;
private:
	void parseLine(std::string_view line) noexcept;

	std::vector<std::pair<std::string, SupportState>> entries;
};

}
