// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_MISC_H
#define SLHELPERS_MISC_H

#include <charconv>
#include <cstddef>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "String.h"

namespace SlHelpers {

struct CmpVersions {
	bool operator()(std::string_view ver1, std::string_view ver2) const
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
	static unsigned int getSublevel(std::string_view s) {
		const auto off = s.starts_with("rc") ? 2U : 0U;
		unsigned int i = 0;
		std::from_chars(s.data() + off, s.data() + s.size(), i);
		return i;
	}
};

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
