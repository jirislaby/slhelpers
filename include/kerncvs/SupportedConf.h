// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../helpers/Enum.h"

namespace SlKernCVS {

#define SUPPORT_STATE(X) \
	X(NonPresent, -3) \
	X(Unsupported, -2) \
	X(UnsupportedOptional, -1) \
	X(Unspecified, 0) \
	X(Supported, 1) \
	X(BaseSupported, 2) \
	X(ExternallySupported, 3) \
	X(KMPSupported, 4)

/**
 * @brief Parses supported.conf and holds/retrieves the information
 */
class SupportedConf {
public:
	/// @brief Level of support for a module
	enum SupportState {
#define EXP(x, v) x = v,
		SUPPORT_STATE(EXP)
#undef EXP
		Count,
	};

	/// @brief Get string representation of \p ss
	static constexpr std::string_view getName(SupportState ss) noexcept {
		switch (ss) {
#define EXP(x, v) case x: return #x;
		SUPPORT_STATE(EXP)
#undef EXP
		case Count:
			break;
		}
		return "INVALID";
	}

	/// @brief Range of SupportState
	using SupportStateRange = SlHelpers::EnumRange<SupportState, NonPresent>;

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
