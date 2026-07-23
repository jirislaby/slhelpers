// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

#include "../helpers/String.h"

namespace SlCVEs {

/**
 * @brief Map between CVE and bugzilla numbers
 */
class CveBscMap {
public:
	/// @brief CVE -> Bugzilla mapping
	using Map = std::unordered_map<std::string, std::string, SlHelpers::String::Hash,
		SlHelpers::String::Eq>;

	CveBscMap() = delete;

	CveBscMap(const CveBscMap &) = delete;
	CveBscMap &operator=(const CveBscMap &) = delete;

	/// @brief Move constructor
	CveBscMap(CveBscMap &&) = default;
	/// @brief Move assignment operator
	CveBscMap &operator=(CveBscMap &&) = default;

	/**
	 * @brief Create a new CveBscMap map from \p cve2Bugzilla
	 * @param cve2Bugzilla File to parse
	 */
	CveBscMap(const std::filesystem::path &cve2Bugzilla);

	/**
	 * @brief Get bugzilla number for a CVE \p cve
	 * @param cve CVE number
	 * @return Bugzilla number or an empty string
	 */
	std::string_view getBsc(std::string_view cve) const {
		const auto it = m_cveBscMap.find(cve);
		if (it != m_cveBscMap.cend())
			return it->second;

		return {};
	}


	/**
	 * @brief Get CVE number for a bugzilla \p bsc
	 * @param bsc Bugzilla number
	 * @return CVE number or an empty string
	 */
	std::string_view getCve(std::string_view bsc) const
	{
		const auto it = m_bscCveMap.find(bsc);
		if (it != m_bscCveMap.cend())
			return it->second;

		return {};
	}

private:
	Map m_cveBscMap;
	Map m_bscCveMap;
};

}
