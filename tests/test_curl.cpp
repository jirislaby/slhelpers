// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "curl/Curl.h"

#include "helpers.h"

using namespace SlCurl;

namespace {

void test_download(const std::filesystem::path &tmpDir)
{
	static const std::string content("test\nfile\n");
	const auto file = tmpDir / __func__;

	{
		std::ofstream ofs(file);
		assert(ofs);
		ofs << content;
	}

	{
		const auto contentOpt = SlCurl::LibCurl::singleDownload("file://" + file.string());
		assert(contentOpt);
		assert(*contentOpt == content);
	}

	{
		SlCurl::LibCurl c;
		const auto contentOpt = c.download("file://" + file.string());
		assert(contentOpt);
		assert(*contentOpt == content);
	}
}

}

int main()
{
	const auto tmpDir = THelpers::getTmpDir();
	std::cout << tmpDir << '\n';

	test_download(tmpDir);

	std::filesystem::remove_all(tmpDir);

	return 0;
}

