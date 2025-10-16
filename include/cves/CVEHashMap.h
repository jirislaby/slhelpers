// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLCVES_CVEHASHMAP_H
#define SLCVES_CVEHASHMAP_H

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace SlCVEs {

class CVEHashMap {
public:
	enum struct ShaSize {
		Long,
		Short
	};

	CVEHashMap() = delete;
	CVEHashMap(ShaSize shaSize, const std::string &b, unsigned y, bool r) :
		shaSize(shaSize), branch(b), year(y), rejected(r) {}

	bool load(const std::filesystem::path &vsource);

	std::string get_cve(const std::string &sha_commit) const {
		const auto it = m_sha_hash_map.find(sha_commit);
		if (it != m_sha_hash_map.cend())
			return it->second;

		return std::string();
	}

	std::vector<std::string> get_shas(const std::string &cve_number) const { //requires (S == ShaSize::Long)
		std::vector<std::string> ret;
		const auto range = m_cve_hash_multimap.equal_range(cve_number);
		for (auto it = range.first; it != range.second; ++it)
			ret.push_back(it->second);
		return ret;
	}

	std::set<std::string> get_all_cves() const { //requires (S == ShaSize::Long)
		std::set<std::string> ret;
		std::transform(m_cve_hash_multimap.cbegin(), m_cve_hash_multimap.cend(),
			       std::inserter(ret, ret.end()),
			       [](const auto &p) {
			return p.first;
		});
		return ret;
	}

private:
	std::unordered_multimap<std::string, std::string> m_cve_hash_multimap;
	std::unordered_map<std::string, std::string> m_sha_hash_map;
	const ShaSize shaSize;
	const std::string branch;
	const unsigned year;
	const bool rejected;
};

}

#endif
