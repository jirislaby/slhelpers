// SPDX-License-Identifier: GPL-2.0-only

#include <INIReader.h>
#include <LDAPConnection.h>

#include "curl/Curl.h"
#include "helpers/Exception.h"

#include "kerncvs/LDAP.h"

using RunEx = SlHelpers::RuntimeException;
using SlHelpers::raise;

using namespace SlKernCVS;

LDAPUsers::UserMap LDAPUsers::getMap()
{
	auto usermapINI = SlCurl::LibCurl::singleDownload("https://kerncvs.suse.de/usermap.ini");
	if (!usermapINI)
		RunEx("Failed to download usermap.ini: ") << SlCurl::LibCurl::lastError() <<
			raise;

	INIReader reader(usermapINI->data(), usermapINI->size());
	if (reader.ParseError())
		RunEx("Failed to parse usermap.ini: ") << reader.ParseErrorMessage() <<
			" (error=" << reader.ParseError() << ')' << raise;

	UserMap userMap;

	static const std::string section {"mail2logname"};
	for (const auto &email: reader.Keys(section)) {
		auto logname = reader.Get(section, email, "");
		if (logname.empty())
			continue;

		auto it = email.find('@');
		if (it == std::string::npos)
			continue;

		auto username = email.substr(0, it);
		if (email != username)
			userMap.emplace(std::move(logname), std::move(username));
	}

	return userMap;
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
