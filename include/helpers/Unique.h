// SPDX-License-Identifier: GPL-2.0-only

#ifndef UNIQUE_H
#define UNIQUE_H

#include <memory>

namespace SlHelpers {

template <typename T>
class Deleter {
public:
	Deleter() = delete;
	Deleter(bool free) : m_free(free) {}

	void operator()(T *ptr) const;

	using Unique = std::unique_ptr<T, Deleter<T>>;
private:
	bool m_free;
};

template <typename T>
struct UniqueHolder : public Deleter<T>::Unique {
	UniqueHolder() : UniqueHolder(nullptr) { }
	UniqueHolder(T *ptr, bool free = true) : Deleter<T>::Unique(ptr, Deleter<T>{free}) {}

	operator T *() { return Deleter<T>::Unique::get(); }
};

}

#endif
