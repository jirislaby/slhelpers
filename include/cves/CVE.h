// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <optional>
#include <string_view>

namespace SlCVEs {

class CVE {
public:
	static std::optional<std::string_view> getCVENumber(const std::string_view &sv) noexcept;
};

}
