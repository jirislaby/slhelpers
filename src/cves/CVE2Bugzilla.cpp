// SPDX-License-Identifier: GPL-2.0-only

#include <cstring>
#include <fstream>
#include <iostream>

#include "helpers/Exception.h"
#include "helpers/String.h"

#include "cves/CVE2Bugzilla.h"

using RunEx = SlHelpers::RuntimeException;
using SlHelpers::raise;

using namespace SlCVEs;

CVE2Bugzilla::CVE2Bugzilla(const std::filesystem::path &cve2bugzilla)
{
	std::ifstream file{cve2bugzilla};

	if (!file.is_open())
		RunEx("Unable to open cve2bugzilla.txt file: ") << cve2bugzilla << ": " <<
			strerror(errno) << raise;

	for (std::string lineS; getline(file, lineS);) {
		std::string_view line(lineS);
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
		const auto cve_number = SlHelpers::String::trim(line.substr(0, cve_end_idx));
		const auto bsc_number = SlHelpers::String::trim(line.substr(bsc_begin_idx + 1));
		if (cve_number.empty() || bsc_number.empty()) {
			std::cerr << cve2bugzilla << ": " << line << '\n';
			continue;
		}
		std::string bug{"bsc#"};
		bug += bsc_number;
		m_cve_bsc_map.emplace(cve_number, bug);
		m_bsc_cve_map.emplace(std::move(bug), cve_number);
	}
}
