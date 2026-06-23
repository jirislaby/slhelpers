// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

class INIReader;

namespace SlKernCVS {

/// @brief Class to fetch LDAP users.
class LDAPUsers {
	using UserSet = std::unordered_set<std::string>;
public:
	LDAPUsers() = delete;
	/**
	 * @brief Construct a new LDAPUsers object.
	 *
	 * @param dn The distinguished name to bind to the LDAP server.
	 * @param password The password for the distinguished name.
	 */
	LDAPUsers(const std::string &dn, const std::string &password);

	/// @brief Get the set of users fetched from LDAP.
	const UserSet &userSet() const { return m_userSet; }

	/// @brief Check if a user is in the set of users fetched from LDAP.
	bool contains(const std::string &key) const noexcept { return m_userSet.contains(key); }
private:
	using UserMap = std::unordered_map<std::string, std::string>;

	UserSet m_userSet;

	static void walkSection(UserMap &userMap, const INIReader &reader,
				const std::string &section, bool lognameIsKey);
	static void addUserMapCond(UserMap &userMap, std::string &&logname, std::string &&email);
	UserMap getMap();
	void buildSet(const std::string &dn, const std::string &password, const UserMap &userMap);

	std::pair<std::string_view, bool>
	getMappedUser(const UserMap &userMap, const std::string &user) const {
		auto it = userMap.find(user);
		if (it != userMap.end())
			return { it->second, true };
		return { user, false };
	}
};

} // namespace
