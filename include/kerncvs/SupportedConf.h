// SPDX-License-Identifier: GPL-2.0-only

#ifndef SUPPORTEDCONF_H
#define SUPPORTEDCONF_H

#include <string>
#include <utility>
#include <vector>

namespace SlKernCVS {

class SupportedConf {
public:
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
	SupportedConf(const std::string &conf);

	SupportState supportState(const std::string &module) const;
private:
	void parseLine(std::string &line);

	std::vector<std::pair<std::string, SupportState>> entries;
};

}

#endif // SUPPORTEDCONF_H
