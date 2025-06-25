#ifndef SUPPORTEDCONF_H
#define SUPPORTEDCONF_H

#include <map>
#include <string>

namespace SlKernCVS {

class SupportedConf {
public:
	using SupportedConfMap = std::map<std::string, int>;

	SupportedConf() = delete;

	static SupportedConfMap parseSupportedConf(const std::string &conf);
private:
	static void parseLine(std::string &line, SupportedConfMap &suppMap);
};

}

#endif // SUPPORTEDCONF_H
