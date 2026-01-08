// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <sstream>
#include <tuple>

namespace SlHelpers {

/**
 * @brief A base class for LastErrorStr and LastErrorStream
 */
template<typename... More>
class LastErrorBase {
	using Tuple = std::tuple<More...>;
public:
	LastErrorBase() noexcept {}

	/// @brief Get n-th error member
	template<size_t idx>
	std::tuple_element_t<idx, Tuple> &get() noexcept {
		return std::get<idx>(m_members);
	}

	/// @brief Get n-th error member
	template<size_t idx>
	const std::tuple_element_t<idx, Tuple> &get() const noexcept {
		return std::get<idx>(m_members);
	}

	/// @brief Set n-th error member
	template<size_t idx, typename Arg>
	void set(Arg &&val) noexcept {
		std::get<idx>(m_members) = std::forward<Arg>(val);
	}
protected:
	/// @brief Wipe out members
	void resetMembers() noexcept {
		std::apply([](auto &... args) { ((args = {}), ...); }, m_members);
	}
private:
	Tuple m_members;
};

/**
 * @brief Stores a string (usually an error string) to be retrieved later
 *
 * This class uses a string internally.
 */
template<typename... More>
class LastErrorStr : public LastErrorBase<More...> {
public:
	/// @brief Wipe out everything
	LastErrorStr &reset() noexcept {
		m_lastError.clear();
		this->resetMembers();
		return *this;
	}

	/**
	 * @brief Store a string into this error
	 * @param str The string to store
	 */
	template <typename T>
	void setError(T &&str)
	requires (std::is_convertible_v<T, std::string_view>) {
		m_lastError = std::forward<T>(str);
	}

	/**
	 * @brief Obtain the stored string
	 * @return The stored string.
	 */
	const std::string &lastError() const & noexcept { return m_lastError; }

private:
	std::string m_lastError;
};

/**
 * @brief Stores a string (usually an error string) to be retrieved later
 *
 * This class uses a stringstream internally, so can be easilly appended.
 */
template<typename... More>
class LastErrorStream : public LastErrorBase<More...> {
public:
	/// @brief Wipe out everything
	LastErrorStream &reset() noexcept {
		m_lastError.str({});
		m_lastError.clear();
		this->resetMembers();
		return *this;
	}

	/**
	 * @brief Store something into this error
	 * @param x What to store
	 * @return This class with \p x stored.
	 */
	template<typename T>
	LastErrorStream &operator<<(const T &x) {
		m_lastError << x;
		return *this;
	}

	/**
	 * @brief Obtain the stored string
	 * @return The stored string.
	 */
	std::string lastError() const noexcept { return m_lastError.str(); }

private:
	std::ostringstream m_lastError;
};

}
