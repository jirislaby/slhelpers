// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <memory>

namespace SlHelpers {

/**
 * @brief The Deleter for UniqueHolder types
 *
 * This class should be instantiated with proper operator() for every UniqueHolder type.
 */
template <typename T>
class Deleter {
public:
	Deleter() = delete;

	/**
	 * @brief Construct a Deleter
	 * @param free Should the pointer be really freed?
	 */
	Deleter(bool free) : m_free(free) {}

	/**
	 * @brief The operator called to free the pointer \p ptr
	 * @param ptr Pointer to memory to be freed
	 */
	void operator()(T *ptr) const;

	/// @brief Deleter and UniqueHolder are just wrappers around std::unique_ptr
	using Unique = std::unique_ptr<T, Deleter<T>>;
private:
	bool m_free;
};

/**
 * @brief Wrapper around std::unique_ptr for easier re-definition of free
 */
template <typename T>
struct UniqueHolder : public Deleter<T>::Unique {
	/// @brief Construct UniqueHolder holding a nullptr
	UniqueHolder() : UniqueHolder(nullptr) { }

	/**
	 * @brief Construct UniqueHolder holding \p ptr
	 * @param ptr Pointer to hold
	 * @param free Should the pointer be freed upon destruction?
	 */
	UniqueHolder(T *ptr, bool free = true) : Deleter<T>::Unique(ptr, Deleter<T>{free}) {}

	/**
	 * @brief Implicit conversion to pointer to T
	 * @return The carried pointer.
	 */
	operator T *() const { return Deleter<T>::Unique::get(); }
};

}
