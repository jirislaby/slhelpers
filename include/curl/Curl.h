// SPDX-License-Identifier: GPL-2.0-only

#ifndef CURL_H
#define CURL_H

#include <filesystem>
#include <optional>
#include <ostream>
#include <string>

typedef void CURL;

namespace SlCurl {

class LibCurl {
public:
	LibCurl();
	~LibCurl();

	bool downloadToStream(const std::string &url, const std::ostream &stream,
			      unsigned *HTTPErrorCode = nullptr);
	bool downloadToFile(const std::string &url, const std::filesystem::path &file,
			    unsigned *HTTPErrorCode = nullptr);
	std::optional<std::string> download(const std::string &url,
					    unsigned *HTTPErrorCode = nullptr);

	static std::optional<std::string> singleDownload(const std::string &url,
							 unsigned *HTTPErrorCode = nullptr);
	static bool singleDownloadToFile(const std::string &url,
					 const std::filesystem::path &file,
					 unsigned *HTTPErrorCode = nullptr);
private:
	CURL *handle;
};

}

#endif // CURL_H
