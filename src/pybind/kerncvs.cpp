// SPDX-License-Identifier: GPL-2.0-only

#include <utility>

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "helpers/Enum.h"

#include "kerncvs/Branches.h"
#include "kerncvs/CollectConfigs.h"
#include "kerncvs/LDAP.h"
#include "kerncvs/Maintainers.h"
#include "kerncvs/Patch.h"
#include "kerncvs/Person.h"
#include "kerncvs/RPMConfig.h"
#include "kerncvs/Stanza.h"
#include "kerncvs/SupportedConf.h"

#include "pybindSupp.h" // IWYU pragma: keep

namespace py = pybind11;
using namespace SlKernCVS;

namespace {

void doBranches(py::module_ &m)
{
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

}

void doCollectConfigs(py::module_ &m)
{
	py::class_<CollectConfigs> cc(m, "CollectConfigs");

	py::enum_<ConfigValue>(cc, "ConfigValue")
		.value("Disabled", ConfigValue::Disabled)
		.value("BuiltIn", ConfigValue::BuiltIn)
		.value("Module", ConfigValue::Module)
		.value("WithValue", ConfigValue::WithValue)
		.export_values();

	cc
		.def(py::init([](const std::string &repoPath, const std::string &rev) {
			return CollectConfigs::create(repoPath, rev);
		}), py::arg("repoPath"), py::arg("rev"), "Parse configs into CollectConfigs")
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
}

void doLDAP(py::module_ &m)
{
	py::class_<LDAPUsers> ldap(m, "LDAPUsers");
	ldap
		.def(py::init([](const std::string &dn, const std::string &password) {
			  return LDAPUsers(dn, password);
			  }),
		    py::arg("dn"), py::arg("password"),
		    "Obtain LDAP users by binding to LDAP with the specified DN and password")
		.def("users",
		     static_cast<const LDAPUsers::UserSet &(LDAPUsers::*)() const &>(&LDAPUsers::userSet),
		     "Obtain users as a set", py::return_value_policy::reference_internal)
		.def("__contains__", &LDAPUsers::contains, py::arg("key"))
		.def("__repr__", [](const LDAPUsers &ldap) {
		     std::stringstream ss;
		     ss << "<LDAP users#=" << ldap.userSet().size() << '>';
		     return ss.str();
		     });
}

void doMaintainers(py::module_ &m)
{
	py::class_<Person> person(m, "Person");

	auto e = py::enum_<RoleType>(person, "RoleType");
	for (auto role: SlHelpers::EnumRange<RoleType>{}) {
		std::string name { Role::toString(role) };
		std::ranges::replace(name, '-', '_');
		e.value(name.c_str(), role);
	}

	person
		.def("role", &Person::role, "Role of the Person")
		.def("name", &Person::name, py::return_value_policy::reference_internal,
		      "Name of the Person")
		.def("user_name", &Person::userName,
		     "User name of the Person")
		.def("email", &Person::email, py::return_value_policy::reference_internal,
		     "E-mail of the Person")
		.def("__repr__", [](const Person &person) {
		     std::stringstream ss;
		     ss << "<Person role=\"" << person.role().toString() <<
			      "\" name=\"" << person.pretty() << "\">";
		     return ss.str();
		     });

	py::class_<Stanza> stanza(m, "Stanza");
	stanza
		.def("name", &Stanza::name, py::return_value_policy::reference_internal,
		     "Name of the Stanza")
		.def("maintainers", &Stanza::maintainers,
		     py::return_value_policy::reference_internal,
		     "List of maintainers in the Stanza")
		.def("empty", &Stanza::empty,
		     "Check if the Stanza has no name, maintainers, and patterns")
		.def("__getitem__", [](const Stanza &stanza, size_t index) {
		     if (index >= stanza.maintainers().size())
			     throw py::index_error();
		     return stanza.maintainers()[index];
		     }, py::return_value_policy::reference_internal)
		.def("__repr__", [](const Stanza &stanza) {
		     std::stringstream ss;
		     ss << "<Stanza name=\"" << stanza.name() <<
			      "\" maintainers#=" << stanza.maintainers().size() << '>';
		     return ss.str();
		     });

	py::class_<Maintainers> maintainers(m, "Maintainers");
	maintainers
		.def(py::init([](const std::filesystem::path &SUSE,
				    const std::filesystem::path &linuxRepo,
				    const std::string &origin) {
			      return Maintainers(SUSE, linuxRepo, origin, [](auto email) {
							 return std::string(email);
						 });
			      }),
		    py::arg("SUSE"), py::arg("linuxRepo"), py::arg("origin") = "origin",
		    "Parse SUSE's and Linux's MAINTAINERS files")
		.def("find_best_match", &Maintainers::findBestMatch, py::arg("paths"),
		     py::return_value_policy::reference_internal,
		     "Find the best matched maintainer from the SUSE's MAINTAINERS file")
		.def("find_best_match_upstream", &Maintainers::findBestMatchUpstream,
		     py::arg("paths"),
		     py::return_value_policy::reference_internal,
		     "Find the best matched maintainer from the Linux's MAINTAINERS file")
		.def("maintainers", &Maintainers::maintainers,
		     py::return_value_policy::reference_internal,
		     "Get all parsed SUSE maintainers")
		.def("upstream_maintainers", &Maintainers::upstream_maintainers,
		     py::return_value_policy::reference_internal,
		     "Get all parsed Linux maintainers")
		.def("suse_users", &Maintainers::suse_users,
		     py::return_value_policy::reference_internal,
		     "Get all met SUSE users")
		.def("__repr__", [](const Maintainers &m) {
		     std::stringstream ss;
		     ss << "<Maintainers SUSE#=" << m.maintainers().size() <<
			      " upstream#=" << m.upstream_maintainers().size() <<
			      " SUSE_users#=" << m.suse_users().size() << '>';
		     return ss.str();
		     });
}

void doPatch(py::module_ &m)
{
	py::class_<Patch> patch(m, "Patch");
	patch
		.def(py::init([](const std::filesystem::path &path) {
			      auto ret = Patch::create(path);
			      if (!ret)
				      throw std::runtime_error(Patch::lastError());
			      return std::move(*ret);
			      }),
		    py::arg("path"), "Parse a patch file")
		.def("header", &Patch::header, "Patch header lines as an array",
		     py::return_value_policy::reference_internal)
		.def("paths", &Patch::paths, "Paths the patch changes",
		     py::return_value_policy::reference_internal)
		.def("__repr__", [](const Patch &patch) {
		     std::stringstream ss;
		     ss << "<Patch header_lines#=" << patch.header().size() <<
			      " paths#=" << patch.paths().size() <<
			      '>';
		     return ss.str();
		     });
}

void doRPMConfig(py::module_ &m)
{
	py::class_<RPMConfig> rpmConf(m, "RPMConfig");
	rpmConf
		.def(py::init([](const std::string &config) {
			      return RPMConfig(config);
			      }),
		    py::arg("config"), "Parses rpm/config.sh")
		.def("__contains__", &RPMConfig::contains, py::arg("key"))
		.def("__getitem__", &RPMConfig::getEx, py::arg("key"),
		     py::return_value_policy::reference_internal)
		.def("__repr__", [](const RPMConfig &) {
		     return "<RPMConfig>";
		     });
}

void doSupportedConf(py::module_ &m)
{
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

} // namespace

PYBIND11_MODULE(slkerncvs, m)
{
	m.doc() = "SlKernCVS – Parse and query files from kerncvs";

	doBranches(m);
	doCollectConfigs(m);
	doLDAP(m);
	doMaintainers(m);
	doPatch(m);
	doRPMConfig(m);
	doSupportedConf(m);
}
