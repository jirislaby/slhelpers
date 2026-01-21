// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <charconv>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "String.h"

namespace SlHelpers {

/**
 * @brief Parse a version string into numbers
 */
struct Version {
	Version() = delete;

	/// @brief Split \p version into a string array
	static constexpr auto versionSplit(std::string_view version) noexcept {
		return String::splitSV(version, ".-");
	}

	/// @brief Convert \p version into a number
	static unsigned versionPart(std::string_view version, bool rc = false) noexcept {
		const auto off = rc && version.starts_with("rc") ? 2U : 0U;
		unsigned int verPart = 0;
		std::from_chars(version.data() + off, version.data() + version.size(), verPart);
		return verPart;
	}

	/**
	 * @brief Sum up version parts as parsed from \p version
	 * @param version Version to parse
	 * @return (Major << 16) | (minor << 8) | sublevel.
	 */
	static unsigned versionSum(std::string_view version) noexcept {
		const auto arr = Version::versionSplit(version);
		unsigned ret = 0;
		for (auto i = 0U; i < 3; ++i) {
			ret <<= 8;
			if (i < arr.size())
				ret += Version::versionPart(arr[i], i == 2);
		}
		return ret;
	}

};

/**
 * @brief Compare versions, to be used as Compare in containers
 */
struct CmpVersions {
	/**
	 * @brief Comparator for versions \p ver1 and \p ver2
	 * @param ver1 One version
	 * @param ver2 Another version
	 * @return true if \p ver1 < \p ver2
	 */
	constexpr bool operator()(std::string_view ver1, std::string_view ver2) const noexcept
	{
		const auto arr1 = Version::versionSplit(ver1);
		const auto arr2 = Version::versionSplit(ver2);
		for (auto i = 0U; i < 2U; ++i) {
			auto ver1 = Version::versionPart(arr1[i]);
			auto ver2 = Version::versionPart(arr2[i]);
			if (ver1 != ver2)
				return ver1 < ver2;
			const auto arr1Last = arr1.size() == i + 1;
			const auto arr2Last = arr2.size() == i + 1;
			if (arr1Last && arr2Last)
				return false;
			if (arr1Last || arr2Last)
				return arr1Last;
		}

		return Version::versionPart(arr1[2], true) < Version::versionPart(arr2[2], true);
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
 * @brief Measure elapsed times
 */
template <typename Rep = double, typename Period = std::milli>
class Measure {
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;
	using Dur = std::chrono::duration<Rep, Period>;
public:
	Measure() noexcept : m_start(Clock::now()) {}

	/// @brief Reset to count from \p point (or now)
	void reset(TimePoint point = Clock::now()) noexcept { m_start = point; }

	/// @brief Returns the duration it took from the construction (or last reset())
	Dur elapsed() const noexcept {
		return std::chrono::duration_cast<Dur>(Clock::now() - m_start);
	}

	/// @brief Returns the duration it took from the construction (or last reset()) and reset
	Dur lap() noexcept {
		auto now = Clock::now();
		auto ret = std::chrono::duration_cast<Dur>(now - m_start);
		reset(now);
		return ret;
	}

	/// @brief Run \p func with \p args and return how long it took
	template <typename Func, typename... Args>
	static Dur profile(Func &&func, Args&&... args) {
		Measure m;
		std::forward<Func>(func)(std::forward<Args>(args)...);
		return m.elapsed();
	}
private:
	TimePoint m_start;
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
