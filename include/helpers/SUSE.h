// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string_view>

#include "String.h"

namespace SlHelpers {

/**
 * @brief Helpers specific to SUSE
 */
struct SUSE {

	/// @brief List of SUSE e-mail domains
	static constexpr std::string_view suseDomains[] = {
		"@suse.com",
		"@suse.cz",
		"@suse.de",
	};

	/**
	 * @brief Evaluate if \p email is likely a SUSE address
	 * @param email E-mail to check
	 * @return True if \p email is likely a SUSE address.
	 */
	static constexpr bool isSUSEAddress(std::string_view email) {
		for (const auto &suseEmail : suseDomains)
			if (String::iEndsWith(email, suseEmail))
				return true;
		return false;
	}
};

}
