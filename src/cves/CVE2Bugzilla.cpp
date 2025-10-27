// SPDX-License-Identifier: GPL-2.0-only

#include <fstream>
#include <iostream>

#include "helpers/String.h"

#include "cves/CVE2Bugzilla.h"

using namespace SlCVEs;

std::optional<CVE2Bugzilla> CVE2Bugzilla::create(const std::filesystem::path &cve2bugzilla)
{
	std::ifstream file{cve2bugzilla};

	if (!file.is_open()) {
		std::cerr << "Unable to open cve2bugzilla.txt file: " << cve2bugzilla << '\n';
		return std::nullopt;
	}

	Map cve_bsc_map;
	Map bsc_cve_map;
	for (std::string line; getline(file, line);) {
		if (line.find("EMBARGOED") != std::string::npos ||
				line.find("BUGZILLA:") == std::string::npos ||
				line.find("CVE") == std::string::npos)
			continue;
		const auto cve_end_idx = line.find_first_of(",");
		const auto bsc_begin_idx = line.find_first_of(":");
		if (cve_end_idx == std::string::npos || cve_end_idx < 10 ||
				bsc_begin_idx == std::string::npos ||
				bsc_begin_idx + 1 >= line.size()) {
			std::cerr << cve2bugzilla << ": " << line << '\n';
			continue;
		}
		const auto cve_number = SlHelpers::String::trim(std::string_view(line).substr(0, cve_end_idx));
		const auto bsc_number = SlHelpers::String::trim(std::string_view(line).substr(bsc_begin_idx + 1));
		if (cve_number.empty() || bsc_number.empty()) {
			std::cerr << cve2bugzilla << ": " << line << '\n';
			continue;
		}
		std::string bug{"bsc#"};
		bug += bsc_number;
		cve_bsc_map.insert(std::make_pair(cve_number, bug));
		bsc_cve_map.insert(std::make_pair(std::move(bug), cve_number));
	}

	return CVE2Bugzilla(std::move(cve_bsc_map), std::move(bsc_cve_map));
}

std::string CVE2Bugzilla::get_bsc(const std::string &cve_number) const
{
	const auto it = m_cve_bsc_map.find(cve_number);
	if (it != m_cve_bsc_map.cend())
		return it->second;

	return {};
}

std::string CVE2Bugzilla::get_cve(const std::string &bsc_number) const
{
	const auto it = m_bsc_cve_map.find(bsc_number);
	if (it != m_bsc_cve_map.cend())
		return it->second;

	return {};
}
