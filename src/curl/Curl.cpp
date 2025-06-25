// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <sstream>

#include <curl/curl.h>

#include "curl/Curl.h"

using namespace SlCurl;

static size_t ssWriter(const char *contents, size_t size, size_t nmemb, std::stringstream *ss)
{
	size *= nmemb;

	ss->write(contents, size);

	return size;
}

LibCurl::LibCurl() : handle(nullptr)
{
	if (curl_global_init(CURL_GLOBAL_ALL)) {
		std::cerr << "Curl: cannot init libcurl\n";
		return;
	}

	handle = curl_easy_init();
	if (!handle) {
		std::cerr << "Curl: failed to get curl_handle\n";
		return;
	}
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, ssWriter);
	curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
}

LibCurl::~LibCurl()
{
	curl_easy_cleanup(handle);
	curl_global_cleanup();
}

std::optional<std::string> LibCurl::download(const std::string &url)
{
	std::stringstream ss;

	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &ss);
	auto ret = curl_easy_perform(handle);
	if (ret != CURLE_OK) {
		long resp = 0;
		curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &resp);
		std::cerr << "Curl: curl_easy_perform() failed (resp=" << resp << "): " <<
			     curl_easy_strerror(ret) << '\n';
		return {};
	}


	return ss.str();
}

std::optional<std::string> LibCurl::singleDownload(const std::string &url)
{
	LibCurl curl;

	return curl.download(url);
}
