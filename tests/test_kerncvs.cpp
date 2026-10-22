// SPDX-License-Identifier: GPL-2.0-only

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <set>
#include <string_view>

#include "git/Commit.h"
#include "git/Repo.h"
#include "helpers/Color.h"
#include "helpers/Misc.h"

#include "kerncvs/Branches.h"
#include "kerncvs/CollectConfigs.h"
#include "kerncvs/LDAP.h"
#include "kerncvs/Patch.h"
#include "kerncvs/PatchesAuthors.h"
#include "kerncvs/RPMConfig.h"
#include "kerncvs/SupportedConf.h"

using namespace SlKernCVS;
using Clr = SlHelpers::Color;

namespace {

void testCollectConfigs(const std::optional<std::filesystem::path> kgit)
{
	if (!kgit) {
		Clr(std::cerr, Clr::YELLOW) << "KSOURCE_GIT not set, skipping " << __func__;
		return;
	}

	auto configs = CollectConfigs::create(*kgit, "origin/stable");

	assert(configs.getConfig("x86_64", "default", "CONFIG_NODES_SHIFT") ==
	       ConfigValue::WithValue);

	try {
		configs.getConfig("x86_64", "default", "CONFIG_NON_EXISTANT");
		assert(false);
	} catch (const std::out_of_range &) {
	}

	{
		auto map = configs.getConfigMap("x86_64", "default");
		assert(!map.contains("CONFIG_NON_EXISTANT"));
		assert(map.contains("CONFIG_HIGHMEM"));
		assert(map.contains("CONFIG_X86_64"));
		assert(map.contains("CONFIG_X86_MSR"));
		assert(map["CONFIG_HIGHMEM"] == ConfigValue::Disabled);
		assert(map["CONFIG_X86_64"] == ConfigValue::BuiltIn);
		assert(map["CONFIG_X86_MSR"] == ConfigValue::Module);
	}
}

void checkBuildSet(const std::set<std::string> &set)
{
	// excluded or non-build
	assert(set.find("master") == set.end());
	assert(set.find("vanilla") == set.end());
	assert(set.find("SLE12-SP4-LTSS") == set.end());
	assert(set.find("scripts") == set.end());

	assert(set.find("SL-16.0-AZURE") != set.end());
	assert(set.find("SL-16.0") != set.end());
	assert(set.find("SLE12-SP5") != set.end());
	assert(set.find("SLE12-SP5-RT") != set.end());
	assert(set.find("cve/linux-5.3-LTSS") != set.end());
}

void testBranches()
{
	static const std::string branchesConf = {
		"master:             build publish	merge:scripts\n"
		"vanilla:            build publish\n"
		"stable:             build publish	merge:scripts merge:-master\n"
		"SL-16.0-AZURE:      build publish	merge:SL-16.0\n"
		"SL-16.0:            build publish	merge:scripts\n"
		"SLE12-SP5:          build		merge:scripts eol:2030-10-31\n"
		"SLE12-SP5-RT:       build		merge:-SLE12-SP5\n"
		"SLE12-SP4-LTSS:\n"
		"cve/linux-5.3-LTSS:      build	merge:scripts\n"
		"scripts:                  publish\n"
	};
	{
		const auto branches = Branches::getBuildBranches(branchesConf);

		for (const auto &b : branches)
			std::cerr << b << '\n';

		checkBuildSet({ branches.begin(), branches.end() });
	}
	const auto branches = Branches::create(branchesConf);
	{
		const auto build = branches.filter(Branches::BUILD);
		checkBuildSet({ build.begin(), build.end() });
	}
	{
		const auto excluded = branches.filter(Branches::EXCLUDED, 0);
		std::set<std::string> set { excluded.begin(), excluded.end() };

		assert(set.find("master") != set.end());
		assert(set.find("vanilla") != set.end());

		assert(set.find("SLE12-SP4-LTSS") == set.end());
	}
	{
		const auto nonBuild = branches.filter(Branches::ANY, Branches::BUILD);
		std::set<std::string> set { nonBuild.begin(), nonBuild.end() };

		assert(set.find("scripts") != set.end());
		assert(set.find("SLE12-SP4-LTSS") != set.end());

		assert(set.find("master") == set.end());
		assert(set.find("SL-16.0") == set.end());
	}
	{
		auto merges = branches.merges("stable");
		std::set<std::string> set { merges.begin(), merges.end() };
		assert(set.size() == 2);
		assert(set.find("scripts") != set.end());
		assert(set.find("master") != set.end());
	}
	{
		const auto set = branches.mergesClosure("SLE12-SP5-RT");
		assert(set.size() == 2);
		assert(set.find("SLE12-SP5") != set.end());
		assert(set.find("scripts") != set.end());
	}
	{
		using namespace std::chrono_literals;
		std::cerr << "eol=" << branches.props("SLE12-SP5").eol << "\n";
		assert(branches.props("SLE12-SP5").eol == 2030y/10/31);
	}
}

void testPatch()
{
	static constinit std::string_view header[] = {
		"Some header",
		"Another header",
		" ---", // this shall not terminate the header thanks to the leading space
		"Third header",
	};
	static constinit std::string_view hunks(
			"--- a/a.txt\n"
			"+++ b/a.txt\n"
			"@@ -100,5 +100,5 @@ some fun\n"
			" context1\n"
			" More context\n"
			"-removed line\n"
			"+added line\n"
			" context2\n"
			" context3\n"
			"--- a/b.txt\n"
			"+++ b/c.txt\n");

	std::stringstream ss;
	for (const auto &e: header)
		ss << e << '\n';
	ss << "---\n" << hunks;

	auto p = Patch::create(ss);
	assert(p);

	assert(p->header().size() == std::size(header));
	for (auto i = 0U; i < std::size(header); ++i)
		assert(p->header()[i] == header[i]);

	assert(p->paths().size() == 3);
	assert(p->paths().contains("a.txt"));
	assert(p->paths().contains("b.txt"));
	assert(p->paths().contains("c.txt"));
}

bool pathStartsWith(const std::filesystem::path &path, const std::filesystem::path &prefix)
{
	auto [path_it, prefix_it] = std::ranges::mismatch(path, prefix);

	return prefix_it == prefix.end();
}

void testPatchesAuthors(const std::optional<std::filesystem::path> kgit)
{
	if (!kgit) {
		Clr(std::cerr, Clr::YELLOW) << "KSOURCE_GIT not set, skipping " << __func__;
		return;
	}

	auto repo = SlGit::Repo::open(*kgit);
	assert(repo);

	auto SL16_0 = repo->commitRevparseSingle("origin/SL-16.0");
	assert(SL16_0);

	auto hasJslaby = false;
	auto hasTiwai = false;
	PatchesAuthors PA{*repo, false, false};
	PA.processAuthors(*SL16_0, [](const std::string &/*email*/) {
		return true;
	}, [&hasJslaby, &hasTiwai](const std::string &email, std::filesystem::path &&file,
	      unsigned /*gitFixes*/, unsigned /*realFixes*/) {
		if (email.starts_with("jslaby@") && pathStartsWith(file, "drivers/tty"))
			hasJslaby = true;
		if (email.starts_with("tiwai@") && pathStartsWith(file, "sound"))
			hasTiwai = true;
		return true;
	});

	assert(hasJslaby);
	assert(hasTiwai);
}

void testRPMConfig()
{
	RPMConfig c("SRCVERSION=6.18\n"
		    "# comment\n"
		    "VARIANT=  \n"
		    "  # COMMENTED=1\n"
		    "COMPRESS_MODULES=\"zstd\"\n"
		    "  \t BUILD_DTBS =  \" Yes\"  \n"
		    "LIVEPATCH=livepatch\n");
	{
		auto val = c.get("SRCVERSION");
		assert(val);
		assert(val->get() == "6.18");
	}
	{
		assert(c.contains("VARIANT"));
		auto val = c.get("VARIANT");
		assert(val);
		assert(val->get() == "");
	}
	{
		auto val = c.get("COMPRESS_MODULES");
		assert(val);
		assert(val->get() == "zstd");
		assert(c["COMPRESS_MODULES"] == "zstd");
	}
	{
		auto val = c.get("BUILD_DTBS");
		assert(val);
		assert(val->get() == " Yes");
	}
	{
		auto val = c.get("LIVEPATCH");
		assert(val);
		assert(val->get() == "livepatch");
	}
	assert(!c.contains("NON_EXISTANT"));
	assert(!c.get("NON_EXISTANT2"));
	assert(!c.contains("COMMENTED"));
}

void testSupportedConf()
{
	static const std::string supportedConf = {
		"#\n"
		"# comment comment\n"
		"-	drivers/input/joystick/*\n"
		"  drivers/mtd/spi-nor/spi-nor # comment\n"
		"- drivers/mtd/*\n"
		"- drivers/usb/typec/altmodes/typec_displayport.ko\n"
		"+ocfs2-kmp      fs/ocfs2/ocfs2_stackglue\n"
		"+base           drivers/net/ethernet/8390/8390\n"
		"-!optional      drivers/ata/ahci_imx\n"
		"+external       arch/powerpc/platforms/powernv/opal-prd\n"
	};

	assert(getName(SupportState::NonPresent) == "NonPresent");

	{
		SupportStateRange range;

		assert(range.begin() != range.end());
		assert(*range.begin() == SupportState::NonPresent);
		auto cnt = 0U;
		for ([[maybe_unused]] const auto &_: range)
			cnt++;
		assert(cnt == 8);
	}

	const SupportedConf supp { supportedConf };

	assert(supp.supportState("non_existing") == SupportState::NonPresent);

	assert(supp.supportState("drivers/input/joystick/something.ko") ==
	       SupportState::Unsupported);
	assert(supp.supportState("drivers/mtd/spi-nor/spi-nor") == SupportState::Unspecified);
	assert(supp.supportState("drivers/mtd/something_else") == SupportState::Unsupported);
	assert(supp.supportState("drivers/usb/typec/altmodes/typec_displayport") ==
	       SupportState::Unsupported);
	assert(supp.supportState("fs/ocfs2/ocfs2_stackglue") == SupportState::KMPSupported);
	assert(supp.supportState("drivers/net/ethernet/8390/8390") == SupportState::BaseSupported);
	assert(supp.supportState("drivers/ata/ahci_imx") == SupportState::UnsupportedOptional);
	assert(supp.supportState("arch/powerpc/platforms/powernv/opal-prd") ==
	       SupportState::ExternallySupported);
}

std::string generatePatch(std::string_view ref, std::string_view ack,
			  const std::vector<std::string_view> &files,
			  std::string_view subsys = {})
{
	std::ostringstream ss;
	ss <<	"From ec50ec378e3fd83bde9b3d622ceac3509a60b6b5 Mon Sep 17 00:00:00 2001\n"
		"From: Some Author <author@domain.org>\n"
		"Date: Thu, 10 Jul 2025 05:57:26 -0700\n"
		"Subject: [PATCH] ipmi: Use dev_warn_ratelimited() for incorrect message warnings\n"
		"Git-commit: ec50ec378e3fd83bde9b3d622ceac3509a60b6b5\n"
		"Patch-mainline: v6.17-rc1\n"
		"References: " << ref << "\n"
		"\n"
		"Some desc\n"
		"\n"
		"More reasons\n"
		"continue here.\n"
		"\n"
		"Signed-off-by: Some Author <author@domain.org>\n"
		"Message-id: <some_20250710_id@domain.org>\n"
		"Signed-off-by: Some Signer <signer@domain.net>\n"
		"Acked-by: Some User   \t<" << ack << ">";
	if (!subsys.empty())
		ss << "\t[" << subsys << ']';
	ss << "\n"
		"---\n";
	for (const auto &file: files) {
		ss <<	"--- a/" << file << "\n"
			"+++ b/" << file << "\n"
			"@@ some diff\n";
	}
	return ss.str();
}

} // namespace

namespace SlKernCVS {

void testLDAP()
{
	static constexpr const std::string_view ini {
		"[logname2obs]\n"
		"; comment\n"
		"; comment2\n"
		"a = b\n"
		"b = c.d\n"
		"\n"
		"[logname2mail]\n"
		"; logname@suse.de assumed\n"
		"lmuser = lm.user@suse.com\n"
		"kernelbugs = kernel-bugs@suse.de\n"
		"kpm = kernel-cvs@suse.de\n"
		"  kss\t =   kernel-cvs@suse.de  \n"
		"self = self@suse.de\n"
		"noat = noatsign\n"
		"\n"
		"[mail2logname]\n"
		"; part before @ assumed\n"
		"mluser@suse.com = ml\n"
		"ml2user@suse.com = ml2\n"
		"id@suse.com = id\n"
	};

	{
		auto map = SlKernCVS::LDAPUsers::parseMap(ini);
		assert(!map.contains("a"));
		assert(!map.contains("b"));
		assert(!map.contains("c.d"));

		assert(map["lmuser"] == "lm.user");
		assert(map["kernelbugs"] == "kernel-bugs");
		assert(map["kpm"] == "kernel-cvs");
		assert(map["kss"] == "kernel-cvs");
		assert(map["ml"] == "mluser");
		assert(map["ml2"] == "ml2user");

		// logname == username derived from email -> not added
		assert(!map.contains("self"));
		// email has no '@' -> not added
		assert(!map.contains("noat"));
		// logname == username derived from email (mail2logname direction) -> not added
		assert(!map.contains("id"));
	}

	assert(SlKernCVS::LDAPUsers::parseMap("").empty());

	// addUserMapCond() exercised directly
	{
		LDAPUsers::UserMap map;
		LDAPUsers::addUserMapCond(map, "user", "no-at-sign");
		assert(map.empty());

		LDAPUsers::addUserMapCond(map, "same", "same@suse.de");
		assert(map.empty());

		LDAPUsers::addUserMapCond(map, "other", "diff@suse.de");
		assert(map["other"] == "diff");
	}

	// malformed entries must be rejected
	for (const auto badIni : { "[logname2mail]\nno-equal-sign\n",
				   "[logname2mail]\n = emptykey@suse.de\n",
				   "[logname2mail]\nemptyval = \n" }) {
		bool threw = false;
		try {
			SlKernCVS::LDAPUsers::parseMap(badIni);
		} catch (const std::runtime_error &) {
			threw = true;
		}
		assert(threw);
	}

	// getMappedUser()
	{
		LDAPUsers::UserMap map { { "kpm", "kernel-cvs" } };

		auto [mapped, found] = LDAPUsers::getMappedUser(map, "kpm");
		assert(found);
		assert(mapped == "kernel-cvs");

		auto [unmapped, notFound] = LDAPUsers::getMappedUser(map, "unknown");
		assert(!notFound);
		assert(unmapped == "unknown");
	}
}

void testProcessPatch()
{
	static constexpr std::string_view patch("some.patch");
	static std::string ack("someone@suse.cz");
	static std::string file("file.c");
	static std::string file2("file2.c");

	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("stable-fixes", "noone@nowhere.com", {file}));
		assert(PA.m_emailFileCountMap.size() == 0);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("stable-fixes", ack, {file}));
		assert(PA.m_emailFileCountMap[ack][file].fixes == 1);
		assert(PA.m_emailFileCountMap[ack][file].realFixes == 0);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("git-fixes", ack, {file}));
		assert(PA.m_emailFileCountMap[ack][file].fixes == 1);
		assert(PA.m_emailFileCountMap[ack][file].realFixes == 0);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("bsc#123456", ack, {file}));
		assert(PA.m_emailFileCountMap[ack][file].fixes == 1);
		assert(PA.m_emailFileCountMap[ack][file].realFixes == 1);
	}
	{
		PatchesAuthors PA;

		PA.processPatch(patch, generatePatch("bsc#123456", ack, {file}, "tty"));
		assert(PA.m_emailFileCountMap[ack][file].fixes == 1);
		assert(PA.m_emailFileCountMap[ack][file].realFixes == 1);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("bsc#123456", ack, {file, file2}));
		assert(PA.m_emailFileCountMap[ack][file].fixes == 1);
		assert(PA.m_emailFileCountMap[ack][file].realFixes == 1);
		assert(PA.m_emailFileCountMap[ack][file2].fixes == 1);
		assert(PA.m_emailFileCountMap[ack][file2].realFixes == 1);
	}
}

} // namespace

int main()
{
	auto kgit = SlHelpers::Env::get<std::filesystem::path>("KSOURCE_GIT");

	testBranches();
	testCollectConfigs(kgit);
	testPatch();
	testPatchesAuthors(kgit);
	testRPMConfig();
	testSupportedConf();

	testLDAP();
	testProcessPatch();

	return 0;
}

