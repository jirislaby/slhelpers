// SPDX-License-Identifier: GPL-2.0-only

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "cves/CVE.h"
#include "cves/CveBscMap.h"
#include "cves/CveShaMap.h"

namespace py = pybind11;
using namespace SlCVEs;

namespace {

void doCve(py::module_ &m)
{
	py::class_<CVE> cve(m, "CVE");
	cve.def_static("get_cve_number", &CVE::getCVENumber, py::arg("cve_string"),
		       "Get CVE number from a CVE string");
}

void doCveBscMap(py::module_ &m)
{
	py::class_<CveBscMap> cveBscMap(m, "CveBscMap");
	cveBscMap
		.def(py::init([](const std::filesystem::path &cve2Bugzilla) {
			      return CveBscMap(cve2Bugzilla);
			      }),
		     py::arg("cve2Bugzilla"),
		     "A map between CVE numbers and BSC bug numbers")
		.def("get_cve", &CveBscMap::getCve, py::arg("bsc_number"),
		     "Get CVE number for a given BSC bug number",
		     py::return_value_policy::reference_internal)
		.def("get_bsc", &CveBscMap::getBsc, py::arg("cve_number"),
		     "Get BSC bug number for a given CVE number",
		     py::return_value_policy::reference_internal)
		.def("__repr__", [](const CveBscMap &) {
		     return "<CveBscMap>";
		     });
}

void doCveShaMap(py::module_ &m)
{
	py::class_<CveShaMap> cveShaMap(m, "CveShaMap");
	py::enum_<CveShaMap::ShaSize>(cveShaMap, "ShaSize")
		.value("Long", CveShaMap::ShaSize::Long)
		.value("Short", CveShaMap::ShaSize::Short);

	cveShaMap
		.def(py::init([](const std::filesystem::path &vsource,
				 CveShaMap::ShaSize shaSize,
				 const std::string &branch,
				 unsigned year,
				 bool rejected) {
			      return CveShaMap(vsource, shaSize, branch, year, rejected);
			      }),
		     py::arg("vsource") = "",
		     py::arg("sha_size") = CveShaMap::ShaSize::Long,
		     py::arg("branch") = "origin/master",
		     py::arg("year") = 0,
		     py::arg("rejected") = false,
		     "A map between CVE numbers and upstream SHAs")
		.def("get_cve", &CveShaMap::getCve, py::arg("sha_commit"),
		     "Get CVE number for a given upstream SHA",
		     py::return_value_policy::reference_internal)
		.def("get_shas", &CveShaMap::getShas, py::arg("cve_number"),
		     "Get upstream SHAs for a given CVE number")
		.def("get_all_cves", &CveShaMap::getAllCves,
		     "Get all CVE numbers in the map")
		.def("__repr__", [](const CveShaMap &map) {
		     std::stringstream ss;
		     ss << "<CveShaMap cves#=" << map.getAllCves().size() << '>';
		     return ss.str();
		     });
}

} // namespace

PYBIND11_MODULE(slcves, m)
{
	m.doc() = "SlCVEs – Python bindings for SlCVEs library";

	doCve(m);
	doCveBscMap(m);
	doCveShaMap(m);
}
