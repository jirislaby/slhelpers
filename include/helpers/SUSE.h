// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_SUSE_H
#define SLHELPERS_SUSE_H

#include <string>

namespace SlHelpers {

struct SUSE {
	/**
	 * @brief Returns if \p email is likely and SUSE address
	 * @param email E-mail to check
	 * @return true if likely SUSE address
	 */
	static bool isSUSEAddress(const std::string &email) {
		return email.ends_with("@suse.com") ||
		       email.ends_with("@suse.cz") ||
		       email.ends_with("@suse.de");
	}
};

}

#endif
