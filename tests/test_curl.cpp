// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "curl/Curl.h"

#include "helpers.h"

using namespace std::chrono_literals;
using namespace SlCurl;

namespace {

#ifndef HAS_CONNECTION
#define HAS_CONNECTION	0
#endif

std::string writeContentToFile(const std::filesystem::path &file, const std::string &content)
{
	std::ofstream ofs(file, std::ios::out | std::ios::binary);
	assert(ofs);
	ofs << content;
	return content;
}

void test_download(const std::string &url, const std::string &content)
{
	unsigned resp = ~0U;

	{
		const auto contentOpt = LibCurl::singleDownload("file:///012345test", &resp);
		std::cerr << __func__ << ": EXPECTED error: " << LibCurl::lastError() << '\n';
		assert(LibCurl::lastError().find("Could not read a file://") != std::string::npos);
		assert(!resp);
		assert(!contentOpt);
	}
#if HAS_CONNECTION != 0
	{
		const auto contentOpt = LibCurl::singleDownload("http://www.google.com", &resp);
		assert(resp >= 200 && resp < 400);
		assert(contentOpt);
	}

	{
		const auto contentOpt = LibCurl::singleDownload("https://www.google.com", &resp);
		assert(resp >= 200 && resp < 400);
		assert(contentOpt);
	}

	{
		const auto contentOpt = LibCurl::singleDownload("http://nonexistant123.cz", &resp);
		std::cerr << __func__ << ": EXPECTED error: " << LibCurl::lastError() << '\n';
		assert(LibCurl::lastError().find("Could not resolve hostname") !=
				std::string::npos);
		assert(!resp);
		assert(!contentOpt);
	}

	{
		const auto contentOpt = LibCurl::singleDownload("https://www.seznam.cz/nonexistant123",
									&resp);
		assert(resp);
	}
#endif
	{
		const auto contentOpt = LibCurl::singleDownload(url, &resp);
		assert(!resp);
		assert(contentOpt);
		assert(*contentOpt == content);
	}

	{
		LibCurl c;
		const auto contentOpt = c.download(url);
		assert(contentOpt);
		assert(*contentOpt == content);
	}
}

void test_downloadToFile(const std::filesystem::path &tmpDir, const std::string &url,
			 const std::string &content)
{
	const auto destFile = tmpDir / (std::string(__func__) + "_dest");
	assert(LibCurl::singleDownloadToFile(url, destFile));

	std::ifstream ifs(destFile, std::ios::in | std::ios::binary);
	assert(ifs);

	std::ostringstream oss;
	oss << ifs.rdbuf();
	assert(oss.str() == content);
}


void test_isDownloadNeeded(const std::filesystem::path &tmpDir)
{
	std::filesystem::path tmp_file = tmpDir / __func__;
	bool exists = false;

	assert(LibCurl::isDownloadNeeded(tmp_file, exists, true, 1h));
	assert(!exists);
	assert(LibCurl::isDownloadNeeded(tmp_file, exists, false, 1h));
	assert(!exists);

	std::ofstream ofs{tmp_file};
	assert(LibCurl::isDownloadNeeded(tmp_file, exists, true, 1h));
	assert(exists);
	assert(!LibCurl::isDownloadNeeded(tmp_file, exists, false, 1h));
	assert(exists);

	std::filesystem::last_write_time(tmp_file,
					 std::filesystem::file_time_type::clock::now() - 2h);
	assert(LibCurl::isDownloadNeeded(tmp_file, exists, false, 1h));
	assert(!LibCurl::isDownloadNeeded(tmp_file, exists, false, 3h));
}

void test_fetchFileIfNeeded()
{
	if (!HAS_CONNECTION)
		return;
	const auto file = LibCurl::fetchFileIfNeeded("trial",
						     "https://www.google.com/robots.txt",
						     false, false, 1h);
	assert(file == "trial");
}

}

int main()
{
	const auto tmpDir = THelpers::getTmpDir();
	std::cout << tmpDir << '\n';

	const auto file = tmpDir / __func__;
	const auto url = "file://" + file.string();
	const auto content = writeContentToFile(file, "test\nfile\n");

	test_download(url, content);
	test_downloadToFile(tmpDir, url, content);

	test_isDownloadNeeded(tmpDir);
	test_fetchFileIfNeeded();

	std::filesystem::remove_all(tmpDir);

	return 0;
}

