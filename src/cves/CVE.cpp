// SPDX-License-Identifier: GPL-2.0-only

#include <cctype>

#include "cves/CVE.h"

using namespace SlCVEs;

std::optional<std::string_view> CVE::getCVENumber(const std::string_view &sv) noexcept
{
	if (sv.length() < std::string_view("CVE-2025-1").length())
		return std::nullopt;

	static constexpr std::string_view CVE("CVE-");
	if (!sv.starts_with(CVE))
		return std::nullopt;

	auto endYear = CVE.length() + 4;
	for (auto pos = CVE.length(); pos < endYear; ++pos)
		if (!std::isdigit(sv[pos]))
			return std::nullopt;

	if (sv[endYear] != '-' || !std::isdigit(sv[endYear + 1]))
		return std::nullopt;

	return sv.substr(0, sv.find_first_not_of("0123456789", endYear + 1));
}
