// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <charconv>
#include <cstddef>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "String.h"

namespace SlHelpers {

/**
 * @brief Compare versions, to be used as Compare in containers
 */
struct CmpVersions {
	constexpr bool operator()(std::string_view ver1, std::string_view ver2) const noexcept
	{
		const auto arr1 = String::splitSV(ver1, ".-");
		const auto arr2 = String::splitSV(ver2, ".-");
		for (auto i = 0U; i < 2U; ++i) {
			unsigned int ver1 = 0;
			unsigned int ver2 = 0;
			std::from_chars(arr1[i].data(), arr1[i].data() + arr1[i].size(), ver1);
			std::from_chars(arr2[i].data(), arr2[i].data() + arr2[i].size(), ver2);

			if (ver1 != ver2)
				return ver1 < ver2;
			const auto arr1Last = arr1.size() == i + 1;
			const auto arr2Last = arr2.size() == i + 1;
			if (arr1Last && arr2Last)
				return false;
			if (arr1Last || arr2Last)
				return arr1Last;
		}

		return getSublevel(arr1[2]) < getSublevel(arr2[2]);
	}

private:
	static unsigned int getSublevel(std::string_view s) noexcept {
		const auto off = s.starts_with("rc") ? 2U : 0U;
		unsigned int i = 0;
		std::from_chars(s.data() + off, s.data() + s.size(), i);
		return i;
	}
};

/**
 * @brief A helper for environment variables
 */
struct Env {
	/**
	 * @brief Get value of \p name in environment
	 * @param name Name of variable to obtain value of
	 * @return Value of environment variable \p name. nullopt if not present.
	 */
	template <typename T = std::string>
	static std::optional<T> get(const std::string &name) noexcept {
		if (const auto env = std::getenv(name.c_str()))
			return env;
		return std::nullopt;
	}
};

/**
 * @brief A helper to convert units
 */
struct Unit {
	/**
	 * @brief Convert \p bytes into human readable form (1 Kib, 20.5 MiB, ...)
	 * @param bytes Value to convert
	 * @param precision Count of numbers after a dot
	 * @param fixed If std::fixed should be used
	 * @return Human readable form of \p bytes.
	 */
	static std::string human(const size_t bytes, const unsigned precision = 2,
				 const bool fixed = true) noexcept {
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
