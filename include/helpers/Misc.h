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
	bool operator()(const std::string &ver1, const std::string &ver2) const
	{
		const auto arr1 = String::split(ver1, ".-");
		const auto arr2 = String::split(ver2, ".-");
		for (auto i = 0U; i < 2U; ++i) {
			const unsigned int ver1 = std::stoi(arr1[i]);
			const unsigned int ver2 = std::stoi(arr2[i]);
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
	static unsigned int getSublevel(const std::string &s) {
		if (String::startsWith(s, "rc")) {
			unsigned int i;
			std::from_chars(s.data() + 2, s.data() + s.size(), i);
			return i;
		} else
			return std::stoi(s);
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
