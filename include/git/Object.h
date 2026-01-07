// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>

#include <git2.h>

#include "../helpers/Unique.h"

#include "Helpers.h"

namespace SlGit {

class Repo;

/**
 * @brief Git Object class -- base for Blob, Commit, Tag, Tree
 */
class Object {
public:
	virtual ~Object() = default;

	/// @brief Get OID (SHA) of this Object
	const git_oid *id() const noexcept { return git_object_id(object()); }
	/// @brief Get OID (SHA) of this Object -- as a string
	std::string idStr() const noexcept { return Helpers::oidToStr(*id()); }

	/// @brief Get Type of this Object
	git_object_t type() const noexcept { return git_object_type(object()); }
	/// @brief Get Type of this Object -- as a string
	std::string typeStr() const noexcept { return git_object_type2string(type()); }

	/// @brief Get the libgit2's \c git_object pointer of this Object
	virtual git_object *object() const noexcept { return nullptr; }

	/// @brief Get the Repo this Object lives in
	const Repo &repo() const { return *m_repo; }

	/// @brief Compare two Objects (their SHAs)
	bool operator==(const Object &other) const noexcept {
	    if (object() == other.object())
		    return true;
	    if (!object() || !other.object())
		    return false;
	    return git_oid_equal(id(), other.id());
	}

	/// @brief Compare two Objects (their SHAs)
	bool operator!=(const Object &other) const noexcept { return !(*this == other); }
protected:
	Object() = delete;
	/// @brief Constuct a new Object
	explicit Object(const Repo &repo) : m_repo(&repo) {}
private:
	const Repo *m_repo;
};

/**
 * @brief Git Object class -- base for Blob, Commit, Tag, Tree
 *
 * This one contains the real libgit2 pointer to one of the above.
 */
template<typename T>
class TypedObject : public Object {
	using Holder = SlHelpers::UniqueHolder<T>;
public:
	TypedObject() = delete;
	/// @brief Get a pointer to the generic git_object
	git_object *object() const noexcept override {
		return reinterpret_cast<git_object *>(typed());
	}

	/// @brief Alias for typed()
	operator T *() const noexcept { return typed(); }

protected:
	/// @brief Construct a new TypedObject
	explicit TypedObject(const Repo &repo, T* typed) noexcept :
		Object(repo), m_typed(typed) { }

	/// @brief Get the stored pointer typed to one of libgit2's types
	T *typed() const noexcept { return m_typed.get(); }
private:
	Holder m_typed;
};

}
