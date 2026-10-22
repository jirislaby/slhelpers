// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "../helpers/String.h"

namespace SlKernCVS {

/// @brief Class to fetch LDAP users.
class LDAPUsers {
public:
	/// @brief Set of users.
	using UserSet = std::unordered_set<std::string, SlHelpers::String::Hash,
	      SlHelpers::String::Eq>;

	LDAPUsers() = delete;
	/**
	 * @brief Construct a new LDAPUsers object.
	 *
	 * @param dn The distinguished name to bind to the LDAP server.
	 * @param password The password for the distinguished name.
	 */
	LDAPUsers(const std::string &dn, const std::string &password);

	/// @brief Get the set of users fetched from LDAP.
	const UserSet &userSet() const & { return m_userSet; }

	/// @brief Get the set of users fetched from LDAP (rvalue).
	UserSet userSet() && { return std::move(m_userSet); }

	/// @brief Check if a user is in the set of users fetched from LDAP.
	bool contains(const std::string &key) const noexcept { return m_userSet.contains(key); }
private:
	friend void testLDAP();

	using UserMap = std::unordered_map<std::string, std::string>;

	UserSet m_userSet;

	static void addUserMapCond(UserMap &userMap, std::string_view logname,
				   std::string_view email);
	static UserMap parseMap(std::string_view mapData);
	UserMap getMap();
	void buildSet(const std::string &dn, const std::string &password, const UserMap &userMap);

	static std::pair<std::string_view, bool>
	getMappedUser(const UserMap &userMap, const std::string &user) {
		auto it = userMap.find(user);
		if (it != userMap.end())
			return { it->second, true };
		return { user, false };
	}
};

} // namespace
