// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_SUSE_H
#define SLHELPERS_SUSE_H

#include <string>

#include "String.h"

namespace SlHelpers {

struct SUSE {
	/**
	 * @brief Returns if \p email is likely and SUSE address
	 * @param email E-mail to check
	 * @return true if likely SUSE address
	 */
	static bool isSUSEAddress(const std::string &email) {
		return String::endsWith(email, "@suse.com") ||
		       String::endsWith(email, "@suse.cz") ||
		       String::endsWith(email, "@suse.de");
	}
};

}

#endif
