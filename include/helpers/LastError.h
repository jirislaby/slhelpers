// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_LASTERROR_H
#define SLHELPERS_LASTERROR_H

#include <string>
#include <sstream>

namespace SlHelpers {

class LastError {
public:
	LastError() {}

	LastError &reset() {
	    m_lastError.str({});
	    m_lastError.clear();
	    return *this;
	}

	template<typename T>
	LastError &operator<<(const T &x)
	{
		m_lastError << x;
		return *this;
	}

	std::string lastError() const { return m_lastError.str(); }
private:
	std::ostringstream m_lastError;
};

}

#endif
