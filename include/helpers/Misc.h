// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_MISC_H
#define SLHELPERS_MISC_H

#include <cstddef>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

namespace SlHelpers {

struct Env {
	template <typename T = std::string>
	static std::optional<T> get(const std::string &name) {
		if (const auto env = std::getenv(name.c_str()))
			return env;
		return std::nullopt;
	}
};

struct Unit {
	static std::string human(const size_t bytes, const unsigned precision = 2,
				 const bool fixed = true) {
		static constexpr const std::string_view units[] {
			"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"
		};

		auto unit = 0U;
		auto bytesD = static_cast<double>(bytes);
		while (bytesD >= 1024.) {
			bytesD /= 1024.;
			unit++;
		}

		std::ostringstream s;
		if (fixed)
			s << std::fixed << std::setprecision(precision);
		s << bytesD << ' ' << units[unit];
		return s.str();
	}
};

}

#endif
