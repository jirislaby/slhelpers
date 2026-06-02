// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <chrono>

#include <pybind11/pybind11.h>

namespace pybind11::detail {

/// @brief Casts a std::chrono::year_month_day to a Python datetime.date object.
template <> struct type_caster<std::chrono::year_month_day> {
	/// @brief The necessary boilerplate for casting std::chrono::year_month_day to Python.
	PYBIND11_TYPE_CASTER(std::chrono::year_month_day, _("datetime.date"));
	/// @brief Casts a std::chrono::year_month_day to a Python datetime.date object.
	static handle cast(std::chrono::year_month_day ymd, return_value_policy, handle) {
		return pybind11::module_::import("datetime").attr("date")(
									  int(ymd.year()),
									  unsigned(ymd.month()),
									  unsigned(ymd.day())
									 ).release();
	}
};

}
