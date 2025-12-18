// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string_view>

namespace SlHelpers {

struct SUSE {
	/**
	 * @brief Evaluate if \p email is likely a SUSE address
	 * @param email E-mail to check
	 * @return True if \p email is likely SUSE address.
	 */
	static constexpr bool isSUSEAddress(std::string_view email) {
		return email.ends_with("@suse.com") ||
		       email.ends_with("@suse.cz") ||
		       email.ends_with("@suse.de");
	}
};

}
