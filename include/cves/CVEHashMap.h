// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../helpers/String.h"

namespace SlCVEs {

/**
 * @brief A map between CVE numbers and upstream SHAs
 */
class CVEHashMap {
public:
	/// @brief CVE -> upstream SHA mapping
	using CVEHashMapTy = std::unordered_multimap<std::string, std::string,
	      SlHelpers::String::Hash, SlHelpers::String::Eq>;
	/// @brief Upstream SHA -> CVE mapping
	using SHAHashMapTy = std::unordered_map<std::string, std::string,
	      SlHelpers::String::Hash, SlHelpers::String::Eq>;

	/// @brief Store Long or Short SHAs
	enum struct ShaSize {
		Long,
		Short
	};

	CVEHashMap() = delete;

	CVEHashMap(const CVEHashMap &) = delete;
	CVEHashMap &operator=(const CVEHashMap &) = delete;

	/// @brief Move constructor
	CVEHashMap(CVEHashMap &&) = default;
	/// @brief Move assignment operator
	CVEHashMap &operator=(CVEHashMap &&) = default;

	/**
	 * @brief Create a new CVEHashMap
	 * @param vsource Path to the vulns git repository
	 * @param shaSize Long or Short
	 * @param branch Branch of \p vsource to walk
	 * @param year A specific year to walk or zero
	 * @param rejected Walk published/ or rejected/
	 */
	CVEHashMap(const std::filesystem::path &vsource,
		   ShaSize shaSize, const std::string &branch,
		   unsigned year, bool rejected);

	/**
	 * @brief Get CVE number for \p sha_commit
	 * @param sha_commit Upstream SHA
	 * @return CVE number
	 */
	std::string_view get_cve(std::string_view sha_commit) const {
		const auto it = m_shaHashMap.find(sha_commit);
		if (it != m_shaHashMap.cend())
			return it->second;

		return {};
	}

	/**
	 * @brief Get SHAs for \p cve_number
	 * @param cve_number CVE number
	 * @return Vector of upstream SHAs (possibly empty)
	 */
	std::vector<std::string> get_shas(std::string_view cve_number) const { //requires (S == ShaSize::Long)
		std::vector<std::string> ret;
		const auto range = m_cveHashMap.equal_range(cve_number);
		std::transform(range.first, range.second, std::back_inserter(ret),
			       [](const auto &p) { return p.second; });
		return ret;
	}

	/**
	 * @brief Get all stored CVE numbers
	 * @return Set of CVE numbers
	 */
	std::set<std::string> get_all_cves() const { //requires (S == ShaSize::Long)
		std::set<std::string> ret;
		std::transform(m_cveHashMap.cbegin(), m_cveHashMap.cend(),
			       std::inserter(ret, ret.end()),
			       [](const auto &p) { return p.first; });
		return ret;
	}

private:
	CVEHashMapTy m_cveHashMap;
	SHAHashMapTy m_shaHashMap;
};

}
