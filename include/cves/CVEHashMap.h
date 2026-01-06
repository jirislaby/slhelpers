// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace SlCVEs {

class CVEHashMap {
public:
	using CVEHashMapTy = std::unordered_multimap<std::string, std::string>;
	using SHAHashMapTy = std::unordered_map<std::string, std::string>;

	enum struct ShaSize {
		Long,
		Short
	};

	CVEHashMap() = delete;

	static std::optional<CVEHashMap> create(const std::filesystem::path &vsource,
						ShaSize shaSize, const std::string &branch,
						unsigned year, bool rejected);

	std::string get_cve(const std::string &sha_commit) const {
		const auto it = m_shaHashMap.find(sha_commit);
		if (it != m_shaHashMap.cend())
			return it->second;

		return {};
	}

	std::vector<std::string> get_shas(const std::string &cve_number) const { //requires (S == ShaSize::Long)
		std::vector<std::string> ret;
		const auto range = m_cveHashMap.equal_range(cve_number);
		std::transform(range.first, range.second, std::back_inserter(ret),
			       [](const auto &p) { return p.second; });
		return ret;
	}

	std::set<std::string> get_all_cves() const { //requires (S == ShaSize::Long)
		std::set<std::string> ret;
		std::transform(m_cveHashMap.cbegin(), m_cveHashMap.cend(),
			       std::inserter(ret, ret.end()),
			       [](const auto &p) { return p.first; });
		return ret;
	}

private:
	CVEHashMap(CVEHashMapTy cveMap, SHAHashMapTy shaMap) :
		m_cveHashMap(std::move(cveMap)), m_shaHashMap(std::move(shaMap)) {}

	CVEHashMapTy m_cveHashMap;
	SHAHashMapTy m_shaHashMap;
};

}
