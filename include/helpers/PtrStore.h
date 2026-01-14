// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <concepts>
#include <iostream>
#include <string_view>
#include <utility>

namespace SlHelpers {

/**
 * @brief A store for a pointer which is freed in the destructor using sqlite3_free()
 */
template <typename T, typename Deleter>
class PtrStore {
public:
	PtrStore() noexcept : PtrStore(nullptr) {}
	/// @brief Construct a new PtrStore with pointer set to \p t
	explicit PtrStore(T *t) noexcept : m_ptr(t), m_deleter(Deleter{}) {}
	~PtrStore() { free(); }

	PtrStore(const PtrStore &) = delete;
	PtrStore &operator=(const PtrStore &) = delete;

	/// @brief Move constructor
	PtrStore(PtrStore &&other) noexcept : m_ptr(other.release()) {}
	/// @brief Move assignment
	PtrStore &operator=(PtrStore &&other) noexcept {
		if (this != &other)
			reset(other.release());
		return *this;
	}

	/// @brief Does this instance hold a valid pointer?
	bool valid() const { return m_ptr; }
	/// @brief bool wrapper around valid()
	operator bool() const { return valid(); }
	/// @brief ! wrapper around valid()
	bool operator!() const { return !valid(); }

	/**
	 * @brief Return the current pointer as string
	 * @return The current pointer as a string or empty string if the pointer is not valid.
	 */
	std::string_view str() const noexcept
	requires std::same_as<T, char> { return m_ptr ? : ""; }

	/// @brief Get the stored pointer
	const T *get() const { return m_ptr; }
	/// @brief Get the stored pointer
	T *get() { return m_ptr; }
	/// @brief Get the stored pointer
	T *operator*() { return m_ptr; }
	/// @brief Get the stored pointer
	const T *operator*() const { return m_ptr; }

	/// @brief Return the pointer and stop owning it
	T *release() {
		return std::exchange(m_ptr, nullptr);
	}

	/// @brief Set the pointer to \p t
	void reset(T *t = nullptr) {
		if (m_ptr != t) {
			free();
			m_ptr = t;
		}
	}

	/**
	 * @brief Get a pointer to the internal pointer
	 * @return Pointer to the internal pointer.
	 *
	 * Usually to assign to the pointer. The previous pointer is freed if it was valid.
	 */
	T **ptr() noexcept {
		free();
		return &m_ptr;
	}
private:
	void free() {
		m_deleter(m_ptr);
		m_ptr = nullptr;
	}

	T *m_ptr;
	[[no_unique_address]] Deleter m_deleter;
};

template<typename T, typename Deleter>
requires std::same_as<T, char>
std::ostream &operator<<(std::ostream &os, const PtrStore<T, Deleter> &err)
{
	return os << err.str();
}

}
