// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <sstream>
#include <stdexcept>
#include <string_view>

namespace SlHelpers {

/**
 * @brief Class for easier construction of std::runtime_error
 *
 * To be used as:
 * @code
 * RuntimeException("This failed: ") << reason << raise;
 * @endcode
 *
 * It is always necessary to use either SlHelpers::raise, or raise() to throw, or getRE() to obtain
 * the exception.
 */
class [[nodiscard("Exception must be thrown")]] RuntimeException {
public:
	RuntimeException() = default;
	/// @brief Construct new RuntimeException, having \p str as the initial exception string
	RuntimeException(std::string_view str) { m_oss << str; }

	struct ThrowNow {};

	/// @brief Add more to the exception string
	template <typename T>
	RuntimeException &operator<<(const T &msg) {
		m_oss << msg;
		return *this;
	}

	/// @brief Return the stored exception string
	std::string str() const { return m_oss.str(); }

	/// @brief Create and return std::runtime_error
	auto getRE() const { return std::runtime_error(str()); }

	/// @brief Create and throw std::runtime_error
	[[noreturn]] void raise() const { throw getRE(); }

	/// @brief Create and throw std::runtime_error
	[[noreturn]] void operator<<(const ThrowNow &) { raise(); }

private:
	std::ostringstream m_oss;
};

inline constexpr RuntimeException::ThrowNow raise;

}
