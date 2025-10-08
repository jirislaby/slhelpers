// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "curl/Curl.h"

#include "helpers.h"

using namespace SlCurl;

namespace {

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
		const auto contentOpt = SlCurl::LibCurl::singleDownload("file:///012345test",
									&resp);
		assert(!resp);
		assert(!contentOpt);
	}
#ifdef HAS_CONNECTION
	{
		const auto contentOpt = SlCurl::LibCurl::singleDownload("http://www.google.com",
									&resp);
		assert(resp >= 200 && resp < 400);
		assert(contentOpt);
	}

	{
		const auto contentOpt = SlCurl::LibCurl::singleDownload("https://www.google.com",
									&resp);
		assert(resp >= 200 && resp < 400);
		assert(contentOpt);
	}

	{
		const auto contentOpt = SlCurl::LibCurl::singleDownload("http://nonexistant123.cz",
									&resp);
		assert(!resp);
		assert(!contentOpt);
	}

	{
		const auto contentOpt = SlCurl::LibCurl::singleDownload("https://www.seznam.cz/nonexistant123",
									&resp);
		assert(resp);
	}
#endif
	{
		const auto contentOpt = SlCurl::LibCurl::singleDownload(url, &resp);
		assert(!resp);
		assert(contentOpt);
		assert(*contentOpt == content);
	}

	{
		SlCurl::LibCurl c;
		const auto contentOpt = c.download(url);
		assert(contentOpt);
		assert(*contentOpt == content);
	}
}

void test_downloadToFile(const std::filesystem::path &tmpDir, const std::string &url,
			 const std::string &content)
{
	const auto destFile = tmpDir / (std::string(__func__) + "_dest");
	assert(SlCurl::LibCurl::singleDownloadToFile(url, destFile));

	std::ifstream ifs(destFile, std::ios::in | std::ios::binary);
	assert(ifs);

	std::ostringstream oss;
	oss << ifs.rdbuf();
	assert(oss.str() == content);
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

	std::filesystem::remove_all(tmpDir);

	return 0;
}

