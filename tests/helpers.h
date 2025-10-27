#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <cassert>
#include <filesystem>
#include <optional>

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

class RestoreEnv {
public:
	RestoreEnv() = delete;
	RestoreEnv(const std::string &env) : m_env(env) {
		if (const auto value = std::getenv(env.c_str()))
			m_value = value;
	}
	~RestoreEnv() {
		if (m_value)
			setenv(m_env.c_str(), m_value->c_str(), true);
		else
			unsetenv(m_env.c_str());
	}

	std::string env() const { return m_env; }
	std::optional<std::string> value() const { return m_value; }

	operator bool() const { return m_value.operator bool(); }
private:
	std::string m_env;
	std::optional<std::string> m_value;
};

}

#endif
