// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "Stanza.h"

namespace SlKernCVS {

/**
 * @brief Loads and holds information from both Linux and SUSE MAINTAINERS files.
 *
 * It can be searches via findBestMatch() and findBestMatchUpstream() functions.
 */
class Maintainers {
public:
	using MaintainersType = std::vector<Stanza>;

	/**
	 * @brief Load maintainers info
	 * @param SUSE path to SUSE's MAINTAINERS file
	 * @param linuxRepo path to upstream linux repository (LINUX_GIT)
	 * @param origin name of branch (of LINUX_GIT) to load upstream MAINTAINERS from
	 * @param translateEmail a function translating emails (eg. to map login -> bugzilla)
	 * @return Maintainers if load was successful, otherwise std::nullopt
	 *
	 * \p SUSE is usually fetched from kerncvs.
	 */
	static std::optional<Maintainers> load(const std::filesystem::path &SUSE,
					       const std::filesystem::path &linuxRepo,
					       const std::string &origin,
					       const Stanza::TranslateEmail &translateEmail) {
		Maintainers m;

		if (!m.loadSUSE(SUSE, translateEmail))
			return std::nullopt;
		if (!linuxRepo.empty() && !m.loadUpstream(linuxRepo, origin, translateEmail))
			return std::nullopt;

		return m;
	}

	/**
	 * @brief Find the best matched maintainer from the SUSE's MAINTAINERS file.
	 * @param paths Set of paths for which a maintainers is looked for
	 * @return Stanza if found or nullptr.
	 */
	const Stanza *findBestMatch(const std::set<std::filesystem::path> &paths) const {
		return findBestMatchInMaintainers(m_maintainers, paths);
	}
	/**
	 * @brief Find the best matched maintainer from the Linux's MAINTAINERS file.
	 * @param paths Set of paths for which a maintainers is looked for
	 * @return Stanza if found or nullptr.
	 */
	const Stanza *findBestMatchUpstream(const std::set<std::filesystem::path> &paths) const {
		return findBestMatchInMaintainers(m_upstream_maintainers, paths);
	}

	/**
	 * @brief Get all parsed SUSE maintainers.
	 * @return SUSE maintainers.
	 */
	const MaintainersType &maintainers() const { return m_maintainers; }
	/**
	 * @brief Get all parsed Linux maintainers.
	 * @return Linux maintainers.
	 */
	const MaintainersType &upstream_maintainers() const { return m_upstream_maintainers; }
	/**
	 * @brief Get all met SUSE users.
	 * @return SUSE users met in both MAINTAINERS files.
	 */
	const std::set<std::string> &suse_users() const { return m_suse_users; }
private:
	Maintainers() {}

	bool loadSUSE(const std::filesystem::path &filename,
		      const Stanza::TranslateEmail &translateEmail);
	bool loadUpstream(const std::filesystem::path &lsource, const std::string &origin,
			  const Stanza::TranslateEmail &translateEmail);

	static const Stanza *findBestMatchInMaintainers(const MaintainersType &sl,
							const std::set<std::filesystem::path> &paths);

	MaintainersType m_maintainers;
	MaintainersType m_upstream_maintainers;
	std::set<std::string> m_suse_users;
};

}
