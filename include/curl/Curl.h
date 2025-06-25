// SPDX-License-Identifier: GPL-2.0-only

#ifndef CURL_H
#define CURL_H

#include <optional>
#include <string>

typedef void CURL;

namespace SlCurl {

class LibCurl {
public:
	LibCurl();
	~LibCurl();

	std::optional<std::string> download(const std::string &url);

	static std::optional<std::string> singleDownload(const std::string &url);
private:
	CURL *handle;
};

}

#endif // CURL_H
