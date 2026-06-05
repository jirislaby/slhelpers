// SPDX-License-Identifier: GPL-2.0-only

#include <utility>

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "kerncvs/Branches.h"
#include "kerncvs/CollectConfigs.h"
#include "kerncvs/RPMConfig.h"
#include "kerncvs/SupportedConf.h"

#include "pybindSupp.h" // IWYU pragma: keep

namespace py = pybind11;
using namespace SlKernCVS;

PYBIND11_MODULE(slkerncvs, m)
{
	m.doc() = "SlKernCVS – Parse and query files from kerncvs";

	// ============= Branches =============

	py::class_<BranchProps> branchProps(m, "BranchProps");
	branchProps
		.def_readonly("is_build", &BranchProps::isBuild)
		.def_readonly("is_publish", &BranchProps::isPublish)
		.def_readonly("is_excluded", &BranchProps::isExcluded)
		.def_readonly("eol", &BranchProps::eol)
		.def_readonly("merges", &BranchProps::merges)
		.def("__repr__", [](const BranchProps &props) {
		     std::stringstream ss;
		     ss << "<BranchProps is_build=" << props.isBuild <<
			     " is_publish=" << props.isPublish <<
			     " is_excluded=" << props.isExcluded <<
			     " eol=" << props.eol <<
			     " merges#=" << props.merges.size() << '>';
		     return ss.str();
		     });

	py::class_<Branches> branches(m, "Branches");
	py::enum_<Branches::Filter>(branches, "Filter")
		.value("Build", Branches::Filter::BUILD)
		.value("Publish", Branches::Filter::PUBLISH)
		.value("Excluded", Branches::Filter::EXCLUDED)
		.value("Any", Branches::Filter::ANY)
		.export_values();
	branches
		.def(py::init([]() {
			    auto ret = Branches::create();
			    if (!ret)
				    throw std::runtime_error("Failed to download branches.conf");
			    return std::move(*ret);
			    }),
			    "Download branches.conf and parse it into Branches")
		.def("map", &Branches::map, py::return_value_policy::reference_internal,
		     "Obtain whole branch map")
		.def("filter", &Branches::filter, py::arg("include") = Branches::ANY,
		     py::arg("exclude") = Branches::EXCLUDED,
		     "Obtain BranchesList according to a filter specified by include and exclude")
		.def("props", &Branches::props, py::arg("branch"),
		     py::return_value_policy::reference_internal, "Return BranchProps for branch")
		.def("merges", &Branches::merges, py::arg("branch"),
		     py::return_value_policy::reference_internal,
		     "Immediate branches that the specified branch merges")
		.def("merges_closure", &Branches::mergesClosure, py::arg("branch"),
		     "Closure of branches that the specified branch merges")
		.def_static("get_build_branches", py::overload_cast<>(&Branches::getBuildBranches),
			    "Download branches.conf and convert it to a list of branches which are built")
		.def("__repr__", [](const Branches &branches) {
		     std::stringstream ss;
		     ss << "<Branches branches#=" << branches.map().size() << '>';
		     return ss.str();
		     });

	// ============= CollectConfigs =============
	py::class_<CollectConfigs> CC(m, "CollectConfigs");
	py::enum_<ConfigValue>(CC, "ConfigValue")
		.value("Disabled", ConfigValue::Disabled)
		.value("BuiltIn", ConfigValue::BuiltIn)
		.value("Module", ConfigValue::Module)
		.value("WithValue", ConfigValue::WithValue)
		.export_values();
	CC.def(py::init([](const std::string &repoPath, const std::string &rev) {
			auto ret = CollectConfigs::create(repoPath, rev);
			return ret;
		}), "Parse configs into CollectConfigs")
		.def("get_arch_map", &CollectConfigs::getArchMap,
		     py::return_value_policy::reference_internal,
		     "Obtain arch->flavor->config map")
		.def("get_flavor_map", &CollectConfigs::getFlavorMap, py::arg("arch"),
		     py::return_value_policy::reference_internal,
		     "Obtain flavor->config map for an arch")
		.def("get_config_map", &CollectConfigs::getConfigMap, py::arg("arch"),
		     py::arg("flavor"),
		     py::return_value_policy::reference_internal,
		     "Obtain config map for an arch and flavor")
		.def("get_config", &CollectConfigs::getConfig, py::arg("arch"), py::arg("flavor"),
		     py::arg("config"),
		     py::return_value_policy::reference_internal,
		     "Obtain config for a branch")
		.def("__repr__", [](const CollectConfigs &cc) {
		     return "<CollectConfigs arch#=" + std::to_string(cc.getArchMap().size()) + '>';
		     });

	// ============= RPMConfig =============

	py::class_<RPMConfig> rpmConf(m, "RPMConfig");
	rpmConf.def(py::init([](const std::string &config) {
			      return RPMConfig(config);
			      }),
		    py::arg("config"), "Parses rpm/config.sh")
		.def("__contains__", &RPMConfig::contains, py::arg("key"))
		.def("__getitem__", &RPMConfig::getEx, py::arg("key"),
		     py::return_value_policy::reference_internal)
		.def("__repr__", [](const RPMConfig &) {
		     return "<RPMConfig>";
		     });

	// ============= SupportedConf =============

	py::class_<SupportedConf> suppConf(m, "SupportedConf");
	py::enum_<SupportState>(suppConf, "SupportState")
		.value("NonPresent",           SupportState::NonPresent)
		.value("Unsupported",          SupportState::Unsupported)
		.value("UnsupportedOptional",  SupportState::UnsupportedOptional)
		.value("Unspecified",          SupportState::Unspecified)
		.value("Supported",            SupportState::Supported)
		.value("BaseSupported",        SupportState::BaseSupported)
		.value("ExternallySupported",  SupportState::ExternallySupported)
		.value("KMPSupported",         SupportState::KMPSupported)
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
