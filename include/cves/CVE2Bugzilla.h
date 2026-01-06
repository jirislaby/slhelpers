// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <string>

#include "../helpers/String.h"

namespace SlCVEs {

/**
 * @brief Map between CVE and bugzilla numbers
 */
class CVE2Bugzilla {
public:
	/// @brief CVE -> Bugzilla mapping
	using Map = std::unordered_map<std::string, std::string, SlHelpers::String::Hash,
		SlHelpers::String::Eq>;

	CVE2Bugzilla() = delete;

	/**
	 * @brief Create a new CVE2Bugzilla map from \p cve2bugzilla
	 * @param cve2bugzilla File to parse
	 * @return CVE2Bugzilla or nullopt on failure
	 */
	static std::optional<CVE2Bugzilla> create(const std::filesystem::path &cve2bugzilla) noexcept;

	/**
	 * @brief Get bugzilla number for a CVE
	 * @param cve_number CVE number
	 * @return Bugzilla number or an empty string
	 */
	std::string get_bsc(std::string_view cve_number) const;

	/**
	 * @brief Get CVE number for a bugzilla
	 * @param bsc_number Bugzilla number
	 * @return CVE number or an empty string
	 */
	std::string get_cve(std::string_view bsc_number) const;
private:
	CVE2Bugzilla(Map cve_bsc_map, Map bsc_cve_map) :
		m_cve_bsc_map(std::move(cve_bsc_map)),
		m_bsc_cve_map(std::move(bsc_cve_map)) {}

	Map m_cve_bsc_map;
	Map m_bsc_cve_map;
};

}
