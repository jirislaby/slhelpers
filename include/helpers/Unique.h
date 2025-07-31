// SPDX-License-Identifier: GPL-2.0-only

#ifndef UNIQUE_H
#define UNIQUE_H

#include <memory>

namespace SlHelpers {

template <typename T>
struct Deleter {
	void operator()(T *ptr) const;

	using Unique = std::unique_ptr<T, Deleter<T>>;
};

template <typename T>
struct UniqueHolder : public Deleter<T>::Unique {
	UniqueHolder() : UniqueHolder(nullptr) { }
	UniqueHolder(T *ptr) : Deleter<T>::Unique(ptr) {}

	operator T *() { return Deleter<T>::Unique::get(); }
};

}

#endif
