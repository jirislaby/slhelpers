// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>
#include <string>

#include "../helpers/String.h"

namespace SlCVEs {

class CVE2Bugzilla {
public:
	using Map = std::unordered_map<std::string, std::string, SlHelpers::String::Hash,
		SlHelpers::String::Eq>;

	CVE2Bugzilla() = delete;

	static std::optional<CVE2Bugzilla> create(const std::filesystem::path &cve2bugzilla) noexcept;

	std::string get_bsc(std::string_view cve_number) const;
	std::string get_cve(std::string_view bsc_number) const;
private:
	CVE2Bugzilla(Map cve_bsc_map, Map bsc_cve_map) :
		m_cve_bsc_map(std::move(cve_bsc_map)),
		m_bsc_cve_map(std::move(bsc_cve_map)) {}

	Map m_cve_bsc_map;
	Map m_bsc_cve_map;
};

}
