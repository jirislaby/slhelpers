// SPDX-License-Identifier: GPL-2.0-only

#include <LDAPConnection.h>
#include <iomanip>
#include <string_view>

#include "curl/Curl.h"
#include "helpers/Exception.h"
#include "helpers/String.h"

#include "kerncvs/LDAP.h"

using RunEx = SlHelpers::RuntimeException;
using SlHelpers::raise;

using namespace SlKernCVS;

void LDAPUsers::addUserMapCond(UserMap &userMap, std::string_view logname, std::string_view email)
{
	auto it = email.find('@');
	if (it == std::string::npos)
		return;

	auto username = email.substr(0, it);
	if (logname != username)
		userMap.emplace(logname, username);
}

LDAPUsers::UserMap LDAPUsers::parseMap(std::string_view mapData)
{
	UserMap userMap;
	SlHelpers::GetLine gl(mapData);
	bool interesting = false;
	bool lognameIsKey;

	while (auto line = gl.get()) {
		line = SlHelpers::String::trim(*line);
		if (line->empty())
			continue;

		if (line->front() == ';')
			continue;

		if (line == "[logname2mail]" || line == "[mail2logname]") {
			interesting = true;
			lognameIsKey = (line == "[logname2mail]");
			continue;
		}

		if (line->front() == '[' && line->back() == ']')
			interesting = false;

		if (!interesting)
			continue;

		auto it = line->find('=');
		if (it != std::string_view::npos) {
			auto key = SlHelpers::String::trim(line->substr(0, it));
			auto val = SlHelpers::String::trim(line->substr(it + 1));
			if (!key.empty() && !val.empty()) {
				if (lognameIsKey)
					addUserMapCond(userMap, key, val);
				else
					addUserMapCond(userMap, val, key);
				continue;
			}
		}

		RunEx("Invalid usermap.ini: ") << std::quoted(*line) << raise;
	}

	return userMap;
}

LDAPUsers::UserMap LDAPUsers::getMap()
{
	auto usermapINI = SlCurl::LibCurl::singleDownload("https://kerncvs.suse.de/usermap.ini");
	if (!usermapINI)
		RunEx("Failed to download usermap.ini: ") << SlCurl::LibCurl::lastError() <<
			raise;

	return parseMap(*usermapINI);
}

void LDAPUsers::buildSet(const std::string &dn, const std::string &password, const UserMap &userMap)
{
	LDAPConnection conn("ldaps://labs-ldap-outpost.prg2.suse.org", 0);
	conn.bind(dn, password);
	StringList attrs;
	attrs.add("sAMAccountName");
	attrs.add("employeeNumber");
	auto entries = conn.search("ou=users,dc=suse,dc=com", LDAPConnection::SEARCH_SUB,
				   "(objectClass=*)", attrs);
	while (auto entry = entries->getNext()) {
		auto sAMAccountName = entry->getAttributeByName("sAMAccountName");
		auto employeeNumber = entry->getAttributeByName("employeeNumber");
		if (!sAMAccountName || !employeeNumber)
			continue;

		auto employeeNumberVal = std::stol(*employeeNumber->getValues().begin());
		if (employeeNumberVal < 0)
			continue;

		auto sAMAccountNameVal = *sAMAccountName->getValues().begin();
		m_userSet.emplace(sAMAccountNameVal);
		auto [mappedUser, mapped] = getMappedUser(userMap, sAMAccountNameVal);
		if (mapped)
			m_userSet.emplace(mappedUser);
	}
}

LDAPUsers::LDAPUsers(const std::string &dn, const std::string &password)
{
	try {
		buildSet(dn, password, getMap());
	} catch (const LDAPException &e) {
		RunEx("Failed to fetch from LDAP server: ") << e << raise;
	}
}
