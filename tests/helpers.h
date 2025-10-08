#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <cassert>
#include <filesystem>

namespace THelpers {

static inline std::filesystem::path __getTmpDir(const std::filesystem::path &cppFile,
						const std::string &suffix = "")
{
	std::string s = std::filesystem::temp_directory_path() / cppFile.stem();
	if (!suffix.empty())
		s += "_" + suffix;
	s += ".XXXXXX";
	assert(::mkdtemp(s.data()));
	return s;
}

#define getTmpDir(...) __getTmpDir(__FILE__, "" __VA_ARGS__)

}

#endif
