// SPDX-License-Identifier: GPL-2.0-only

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "kerncvs/SupportedConf.h"

namespace py = pybind11;
using namespace SlKernCVS;

PYBIND11_MODULE(slkerncvs, m)
{
	m.doc() = "SlKernCVS – supported for files from kerncvs";

	py::class_<SupportedConf> suppConf(m, "SupportedConf");

	py::enum_<SupportedConf::SupportState>(suppConf, "SupportState")
		.value("NonPresent",           SupportedConf::SupportState::NonPresent)
		.value("Unsupported",          SupportedConf::SupportState::Unsupported)
		.value("UnsupportedOptional",  SupportedConf::SupportState::UnsupportedOptional)
		.value("Unspecified",          SupportedConf::SupportState::Unspecified)
		.value("Supported",            SupportedConf::SupportState::Supported)
		.value("BaseSupported",        SupportedConf::SupportState::BaseSupported)
		.value("ExternallySupported",  SupportedConf::SupportState::ExternallySupported)
		.value("KMPSupported",         SupportedConf::SupportState::KMPSupported)
		.export_values();

	suppConf
		.def(py::init([](const std::string &conf) {
			      return SupportedConf(conf);
			      }),
		     py::arg("conf"),
		     "Parses supported.conf")
		.def("support_state",
		     &SupportedConf::supportState,
		     py::arg("module"),
		     "Find supported state of module")
		.def("__repr__", [](const SupportedConf &) {
		     return "<SupportedConf>";
		     });
}
