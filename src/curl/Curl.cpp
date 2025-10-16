// SPDX-License-Identifier: GPL-2.0-only

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <curl/curl.h>

#include "curl/Curl.h"
#include "helpers/Color.h"
#include "helpers/HomeDir.h"

using namespace SlCurl;
using Clr = SlHelpers::Color;

static size_t ssWriter(const char *contents, size_t size, size_t nmemb, std::ostream *stream)
{
	size *= nmemb;

	stream->write(contents, size);

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

bool LibCurl::downloadToStream(const std::string &url, const std::ostream &stream,
			       unsigned *HTTPErrorCode)
{
	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &stream);
	auto ret = curl_easy_perform(handle);
	long resp = 0;
	curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &resp);
	if (HTTPErrorCode)
		*HTTPErrorCode = resp;
	if (ret != CURLE_OK) {
		std::cerr << "Curl: curl_easy_perform() failed (resp=" << resp << "): " <<
			     curl_easy_strerror(ret) << '\n';
		return false;
	}

	return true;
}

bool LibCurl::downloadToFile(const std::string &url, const std::filesystem::path &file,
			     unsigned *HTTPErrorCode)
{
	std::ofstream fs(file);
	if (!fs)
		return false;

	return downloadToStream(url, fs, HTTPErrorCode);
}

std::optional<std::string> LibCurl::download(const std::string &url, unsigned *HTTPErrorCode)
{
	std::ostringstream ss;

	if (!downloadToStream(url, ss, HTTPErrorCode))
		return {};

	return ss.str();
}

std::optional<std::string> LibCurl::singleDownload(const std::string &url, unsigned *HTTPErrorCode)
{
	LibCurl curl;

	return curl.download(url, HTTPErrorCode);
}

bool LibCurl::singleDownloadToFile(const std::string &url, const std::filesystem::path &file,
				   unsigned *HTTPErrorCode)
{
	LibCurl curl;

	return curl.downloadToFile(url, file, HTTPErrorCode);
}

bool LibCurl::isDownloadNeeded(const std::filesystem::path &filePath, bool &fileAlreadyExists,
			       bool forceRefresh, const std::chrono::hours &hours)
{
	if (!std::filesystem::exists(filePath))
		return true;

	fileAlreadyExists = true;
	if (forceRefresh)
		return true;

	const auto mtime = std::filesystem::last_write_time(filePath);
	const auto now = std::filesystem::file_time_type::clock::now();

	return mtime < now - hours;
}

std::filesystem::path LibCurl::fetchFileIfNeeded(const std::filesystem::path &filePath,
						 const std::string &url,
						 bool forceRefresh, bool ignoreErrors,
						 const std::chrono::hours &hours)
{
	bool fileAlreadyExists = false;
	if (!isDownloadNeeded(filePath, fileAlreadyExists, forceRefresh, hours))
		return filePath;

	if (forceRefresh)
		std::cout << "Downloading... " << filePath << " from " << url << '\n';

	auto newPath(filePath);
	newPath += ".NEW";
	unsigned http_code;
	if (!singleDownloadToFile(url, newPath, &http_code)) {
		if (ignoreErrors)
			return "";
		Clr(std::cerr, Clr::RED) << "Failed to fetch " << url << " to " << filePath;
		if (fileAlreadyExists)
			return filePath;
		return "";
	}
	if (http_code >= 400) {
		if (ignoreErrors)
			return "";
		Clr(std::cerr, Clr::RED) << "Failed to fetch " << url << " (" << http_code <<
					    ") " << " to " << filePath;
		if (fileAlreadyExists)
			return filePath;
		return "";
	}
	std::error_code ec;
	std::filesystem::rename(newPath, filePath, ec);
	if (ec) {
		Clr(std::cerr, Clr::RED) << "Failed to rename " << newPath << " to " << filePath;
		return "";
	}

	return filePath;
}
