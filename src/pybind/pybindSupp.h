// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <chrono>

#include <pybind11/pybind11.h>

namespace pybind11::detail {

template <> struct type_caster<std::chrono::year_month_day> {
	PYBIND11_TYPE_CASTER(std::chrono::year_month_day, _("datetime.date"));
	static handle cast(std::chrono::year_month_day ymd, return_value_policy, handle) {
		return pybind11::module_::import("datetime").attr("date")(
									  int(ymd.year()),
									  unsigned(ymd.month()),
									  unsigned(ymd.day())
									 ).release();
	}
};

}
