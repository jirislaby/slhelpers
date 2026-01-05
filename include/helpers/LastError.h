// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <sstream>

namespace SlHelpers {

/**
 * @brief Stores a string (usually an error string) to be retrieved later
 */
class LastError {
public:
	LastError() {}

	/**
	 * @brief Store a new string
	 * @return This class after being wiped out.
	 */
	LastError &reset() {
	    m_lastError.str({});
	    m_lastError.clear();
	    return *this;
	}

	/**
	 * @brief Store something into this error
	 * @param x What to store
	 * @return This class with \p x stored.
	 */
	template<typename T>
	LastError &operator<<(const T &x)
	{
		m_lastError << x;
		return *this;
	}

	/**
	 * @brief Obtain the stored string
	 * @return The stored string.
	 */
	std::string lastError() const { return m_lastError.str(); }
private:
	std::ostringstream m_lastError;
};

}
