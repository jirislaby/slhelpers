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
class CveShaMap {
public:
	/// @brief CVE -> upstream SHA mapping
	using CveShaMapTy = std::unordered_multimap<std::string, std::string,
	      SlHelpers::String::Hash, SlHelpers::String::Eq>;
	/// @brief Upstream SHA -> CVE mapping
	using ShaCveMapTy = std::unordered_map<std::string, std::string,
	      SlHelpers::String::Hash, SlHelpers::String::Eq>;

	/// @brief Store Long or Short SHAs
	enum struct ShaSize {
		Long,
		Short
	};

	CveShaMap() = delete;

	CveShaMap(const CveShaMap &) = delete;
	CveShaMap &operator=(const CveShaMap &) = delete;

	/// @brief Move constructor
	CveShaMap(CveShaMap &&) = default;
	/// @brief Move assignment operator
	CveShaMap &operator=(CveShaMap &&) = default;

	/**
	 * @brief Create a new CveShaMap
	 * @param vsource Path to the vulns git repository
	 * @param shaSize Long or Short
	 * @param branch Branch of \p vsource to walk
	 * @param year A specific year to walk or zero
	 * @param rejected Walk published/ or rejected/
	 */
	CveShaMap(const std::filesystem::path &vsource = {},
		  ShaSize shaSize = ShaSize::Long,
		  const std::string &branch = "origin/master",
		  unsigned year = 0,
		  bool rejected = false);

	/**
	 * @brief Get CVE number for \p shaCommit
	 * @param shaCommit Upstream SHA
	 * @return CVE number
	 */
	std::string_view getCve(std::string_view shaCommit) const {
		const auto it = m_shaCveMap.find(shaCommit);
		if (it != m_shaCveMap.cend())
			return it->second;

		return {};
	}

	/**
	 * @brief Get SHAs for \p cveNumber
	 * @param cveNumber CVE number
	 * @return Vector of upstream SHAs (possibly empty)
	 */
	std::vector<std::string> getShas(std::string_view cveNumber) const { //requires (S == ShaSize::Long)
		std::vector<std::string> ret;
		const auto range = m_cveShaMap.equal_range(cveNumber);
		std::transform(range.first, range.second, std::back_inserter(ret),
			       [](const auto &p) { return p.second; });
		return ret;
	}

	/**
	 * @brief Get all stored CVE numbers
	 * @return Set of CVE numbers
	 */
	std::set<std::string> getAllCves() const { //requires (S == ShaSize::Long)
		std::set<std::string> ret;
		std::transform(m_cveShaMap.cbegin(), m_cveShaMap.cend(),
			       std::inserter(ret, ret.end()),
			       [](const auto &p) { return p.first; });
		return ret;
	}

private:
	CveShaMapTy m_cveShaMap;
	ShaCveMapTy m_shaCveMap;
};

}
