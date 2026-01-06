// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>

#include "Pattern.h"
#include "Person.h"

namespace SlKernCVS {

/**
 * @brief Stanza (a subsystem) from MAINTAINERS file.
 *
 * It usually contains maintainer(s) (list of Person) and file/pattern(s) (list of Pattern) they
 * maintain.
 */
class Stanza {
public:
	/// @brief Callback to translate an e-mail
	using TranslateEmail = std::function<std::string (std::string_view sv)>;
	/// @brief Maintainers are a list of Person
	using Maintainers = std::vector<Person>;

	Stanza() = default;
	/**
	 * @brief Construct new Stanza called \p name
	 * @param name Name of the Stanza
	 */
	Stanza(std::string name) : m_name(std::move(name)) {}
	/**
	 * @brief Construct new Stanza called \p n with one Person (having \p name and \p email)
	 * @param n Name of the Stanza
	 * @param name Name of a Person
	 * @param email E-mail of a Person
	 */
	Stanza(std::string n, std::string name, std::string email)
		: m_name(std::move(n)), m_maintainers{Person(Role::Maintainer, std::move(name),
							     std::move(email))} { }

	/**
	 * @brief Return weight of \p path in this Stanza
	 * @param path A path to look for
	 * @return Maximum weight of \p path in this Stanza or 0 if not found.
	 */
	unsigned match_path(const std::filesystem::path &path) const {
		return std::accumulate(m_patterns.cbegin(), m_patterns.cend(), 0u,
				       [&path](unsigned m, const Pattern &p) {
			return std::max(m, p.match(path));
		});
	}

	/**
	 * @brief Add a maintainer (as Maintainer) and store them into SUSE users set too
	 * @param maintainer SUSE maintainer
	 * @param suse_users Set to add to
	 * @param translateEmail Callback to update the Person's e-mail
	 */
	void add_maintainer_and_store(std::string_view maintainer,
				      std::set<std::string> &suse_users,
				      const TranslateEmail &translateEmail) {
		if (auto m = Person::parsePerson(maintainer, Role::Maintainer)) {
			suse_users.insert(m->userName());
			// TODO
			m->setEmail(translateEmail(m->email()));
			// END TODO
			m_maintainers.push_back(std::move(*m));
		} else
			std::cerr << "MAINTAINERS: contact " << maintainer <<
				     " cannot be parsed into name and email!\n";
	}

	/**
	 * @brief Creates a Person and adds it as Maintainer
	 * @param name Name of the Person
	 * @param email E-mail of the Person
	 * @param cnt Count of changes
	 * @param translateEmail Callback to update the Person's e-mail
	 */
	void add_backporter(const std::string &name, std::string_view email,
			    unsigned cnt, const TranslateEmail &translateEmail) {
		m_maintainers.push_back(Person(Role::Maintainer, name,
					       /*TODO*/ translateEmail(email), cnt));
	}

	/**
	 * @brief Add a \p maintainer (as Upstream) if in \p suse_users
	 * @param maintainer Maintainer to add
	 * @param suse_users Set of SUSE users
	 * @param translateEmail Callback to update the Person's e-mail
	 */
	void add_maintainer_if(std::string_view maintainer,
			       const std::set<std::string> &suse_users,
			       const TranslateEmail &translateEmail) {
		if (auto m = Person::parsePerson(maintainer, Role::Upstream)) {
			// TODO
			m->setEmail(translateEmail(m->email()));
			// END TODO
			if (suse_users.contains(m->userName()))
				m_maintainers.push_back(std::move(*m));
		} else
			std::cerr << "Upstream MAINTAINERS: contact " << maintainer <<
				     " cannot be parsed into name and email!\n";
	}

	/**
	 * @brief Add \p pattern to this Stanza
	 * @param pattern Pattern to add
	 * @return true on success.
	 */
	bool add_pattern(std::string pattern) {
		auto p = Pattern::create(std::move(pattern));
		if (!p)
			return false;
		m_patterns.push_back(std::move(*p));
		return true;
	}

	/// @brief Check if this Stanza has no name, maintainers, and patterns
	bool empty() const {
		return m_name.empty() || m_maintainers.empty() || m_patterns.empty();
	}

	/**
	 * @brief Obtain a list of Person in this Stanza
	 * @return List of Person classes.
	 */
	const Maintainers &maintainers() const { return m_maintainers; }

	/**
	 * @brief Reset Stanza and start from the beginning
	 * @param n New name of Stanza
	 *
	 * Called while parsing MAINTAINERS and a new subsystem was parsed.
	 */
	void new_entry(std::string n) {
		m_name = std::move(n);
		m_maintainers.clear();
		m_patterns.clear();
	}

	/// @brief Get name/title of this Stanza
	const std::string &name() const { return m_name; }
private:
	std::string m_name;
	Maintainers m_maintainers;
	std::vector<Pattern> m_patterns;
};

} // namespace
