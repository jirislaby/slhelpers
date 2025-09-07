// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_OBJECT_H
#define GIT_OBJECT_H

#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"

namespace SlGit {

class Repo;

class Object {
protected:
	Object() = default;
public:
	virtual ~Object() = default;

	const git_oid *id() const { return git_object_id(object()); }
	std::string idStr() const { return Helpers::oidToStr(*id()); }

	virtual git_object *object() const = 0;

	bool operator==(const Object &other) const noexcept {
	    if (object() == other.object())
		    return true;
	    if (!object() || !other.object())
		    return false;
	    return git_oid_equal(id(), other.id());
	}

	bool operator!=(const Object &other) const noexcept { return !(*this == other); }
};

template<typename T>
class TypedObject : public Object {
	using Holder = SlHelpers::UniqueHolder<T>;
public:
	TypedObject() = delete;
	git_object *object() const noexcept override {
		return reinterpret_cast<git_object *>(typed());
	}

	operator T *() const { return typed(); }

protected:
	explicit TypedObject(T* typed) : m_typed(typed) { }

	T *typed() const { return m_typed.get(); }

private:
	Holder m_typed;
};

}

#endif
