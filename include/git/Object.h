// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"

namespace SlGit {

class Repo;

class Object {
public:
	virtual ~Object() = default;

	const git_oid *id() const noexcept { return git_object_id(object()); }
	std::string idStr() const noexcept { return Helpers::oidToStr(*id()); }

	git_object_t type() const noexcept { return git_object_type(object()); }
	std::string typeStr() const noexcept { return git_object_type2string(type()); }

	virtual git_object *object() const noexcept { return nullptr; }

	const Repo &repo() const { return *m_repo; }

	bool operator==(const Object &other) const noexcept {
	    if (object() == other.object())
		    return true;
	    if (!object() || !other.object())
		    return false;
	    return git_oid_equal(id(), other.id());
	}

	bool operator!=(const Object &other) const noexcept { return !(*this == other); }
protected:
	Object() = delete;
	explicit Object(const Repo &repo) : m_repo(&repo) {}
private:
	const Repo *m_repo;
};

template<typename T>
class TypedObject : public Object {
	using Holder = SlHelpers::UniqueHolder<T>;
public:
	TypedObject() = delete;
	git_object *object() const noexcept override {
		return reinterpret_cast<git_object *>(typed());
	}

	operator T *() const noexcept { return typed(); }

protected:
	explicit TypedObject(const Repo &repo, T* typed) noexcept :
		Object(repo), m_typed(typed) { }

	T *typed() const noexcept { return m_typed.get(); }
private:
	Holder m_typed;
};

}
