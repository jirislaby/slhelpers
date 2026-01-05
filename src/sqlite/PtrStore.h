// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <iostream>
#include <sqlite3.h>
#include <string_view>

namespace SlSqlite {

/**
 * @brief A store for a pointer which is freed in the destructor using sqlite3_free()
 */
template <typename T>
class PtrStore {
public:
	PtrStore() noexcept : m_ptr(nullptr) {}
	~PtrStore() { free(); }

	PtrStore(const PtrStore &) = delete;
	PtrStore &operator=(const PtrStore &) = delete;

	PtrStore(PtrStore &&other) noexcept : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }
	PtrStore &operator=(PtrStore &&other) noexcept {
		if (this != &other) {
			free();
			std::swap(m_ptr, other.m_ptr);
		}
		return *this;
	}

	/**
	 * @brief Does this instance hold a valid pointer?
	 * @return true if the pointer is non-NULL.
	 */
	bool valid() const { return m_ptr; }
	operator bool() const { return valid(); }
	bool operator!() const { return !valid(); }

	/**
	 * @brief Return the current pointer as string
	 * @return The current pointer as a string or empty string if the pointer is not valid.
	 */
	template <typename U = T,
		  typename = std::enable_if_t<std::is_same_v<U, char>>>
	std::string_view str() const noexcept { return m_ptr ? : ""; }

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
		sqlite3_free(m_ptr);
		m_ptr = nullptr;
	}

	T *m_ptr;
};

template<typename T,
	 typename = std::enable_if_t<std::is_same_v<T, char>>>
std::ostream &operator<<(std::ostream &os, const PtrStore<T> &err)
{
	return os << err.str();
}

using CharPtrStore = PtrStore<char>;

}
