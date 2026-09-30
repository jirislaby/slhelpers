// SPDX-License-Identifier: GPL-2.0-only

#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

#include "curl/Curl.h"

namespace py = pybind11;
using namespace SlCurl;

namespace {

void doCurl(py::module_ &m)
{
	py::class_<LibCurl> curl(m, "LibCurl", "LibCurl class for handling curl operations");
	curl.def_static("fetch_file_if_needed",
			[](const std::filesystem::path &filePath,
			   const std::string &url,
			   bool forceRefresh,
			   bool ignoreErrors,
			   const std::chrono::hours &hours) {
				auto ret = LibCurl::fetchFileIfNeeded(filePath, url, forceRefresh,
								      ignoreErrors, hours);
				if (!ignoreErrors && ret.empty())
					throw std::runtime_error("Failed to fetch file from URL: " +
								 url);
				return ret;
			},
			py::arg("file_path"),
			py::arg("url"),
			py::arg("force_refresh"),
			py::arg("ignore_errors"),
			py::arg("hours"),
			"Fetch a file from a URL if it doesn't exist, is outdated, or if force_refresh is True. Returns the path to the file or raises an exception if the download fails and ignore_errors is False.");
}

} // namespace

PYBIND11_MODULE(slcurl, m)
{
	m.doc() = "SlCurl – Python bindings for SlCurl library";

	doCurl(m);
}
