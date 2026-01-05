// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string>

namespace SlKernCVS {

/**
 * @brief Role of a Person
 *
 * The class converts roles to indices and back too.
 */
class Role {
private:
	static constexpr const std::string_view roleNames[] = {
		"Author",
		"Signed-off-by",
		"Co-developed-by",
		"Suggested-by",
		"Reviewed-by",
		"Acked-by",
		"Tested-by",
		"Reported-by",
		"Maintainer",
		"Upstream"
	};
public:
	enum RoleType {
		Author,
		SignedOffBy, FirstRole = SignedOffBy,
		CoDevelopedBy,
		SuggestedBy,
		ReviewedBy,
		AckedBy, LastRole = AckedBy,
		TestedBy,
		ReportedBy,
		Maintainer,
		Upstream,
		Last
	};
	static_assert(Last == std::size(roleNames));

	Role() = delete;
	Role(size_t index) : m_role(static_cast<RoleType>(index)) {}
	Role(RoleType role) : m_role(role) {}

	RoleType role() const { return m_role; }

	constexpr auto index() const { return static_cast<std::size_t>(m_role); }
	constexpr const auto &toString() const { return roleNames[index()]; }
private:
	RoleType m_role;
};

/**
 * @brief Information about a person (such as username, e-mail, Role)
 */
class Person {
public:
	Person() = delete;

	/**
	 * @brief Create a Person with Role \p r, \p name, \p email and \p count of changes
	 * @param r Role of this Person
	 * @param name Name of this Person
	 * @param email E-mail of this Person
	 * @param count Count of changes historically done by this Person (in $KSOURCE_GIT)
	 */
	Person(Role r, std::string name, std::string email, unsigned count = 0) :
		m_role(std::move(r)), m_name(std::move(name)), m_email(std::move(email)),
		m_count(count) {}

	const Role &role() const { return m_role; }
	const std::string &name() const { return m_name; }
	const std::string &email() const { return m_email; }
	std::string userName() const { return m_email.substr(0, m_email.find("@")); }
	auto count() const { return m_count; }

	/**
	 * @brief Pretty format as "username <e-mail>"
	 * @param includeName If output the username part
	 * @return Either "username <e-mail>" when \p includeName is true or "e-mail" otherwise.
	 */
	std::string pretty(bool includeName = true) const {
		if (includeName && !name().empty())
			return name() + " <" + email() + ">";
		return email();
	}

	/**
	 * @brief Pretty format as "username <e-mail>"
	 * @param translate A translate callback for e-mail
	 * @param includeName If output the username part
	 * @return Either "username <e-mail>" when \p includeName is true or "e-mail" otherwise.
	 *
	 * The e-mail is passed to \p translate() before passing to the output.
	 */
	template <typename T>
	std::string pretty(const T &translate, bool includeName = true) const {
		if (includeName && !name().empty())
			return name() + " <" + translate(email()) + ">";
		return translate(email());
	}

	void setEmail(const std::string &email) { m_email = email; }

	/**
	 * @brief Parse \p src into a Person
	 * @param src Line to parse
	 * @param role Role to set to the new Person
	 * @return Person, or nullopt in case of failure.
	 *
	 * Parses lines like:
	 * M: First LastName <email@somewhere.com>
	 * M: email@somewhere.com
	 */
	static std::optional<Person> parsePerson(std::string_view src, Role role);

	/**
	 * @brief Try to parse a line
	 * @param src Line to parse
	 * @return A Person if successful, std::nullopt otherwise.
	 */
	static std::optional<Person> parse(std::string_view src)
	{
		for (std::size_t i = Role::FirstRole; i <= Role::LastRole; ++i) {
			Role r(i);
			if (src.starts_with(r.toString()))
				if (auto p = parsePerson(src, std::move(r)))
					return p;
		}
		return std::nullopt;
	}
private:
	Role m_role;
	std::string m_name;
	std::string m_email;
	unsigned m_count;

};

} // namespace
