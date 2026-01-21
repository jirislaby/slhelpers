// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>
#include <iostream>
#include <set>

#include "kerncvs/Branches.h"
#include "kerncvs/PatchesAuthors.h"
#include "kerncvs/SupportedConf.h"

using namespace SlKernCVS;

namespace {

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
		"SLE12-SP5:          build		merge:scripts\n"
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

	const SupportedConf supp { supportedConf };

	assert(supp.supportState("non_existing") == SupportedConf::NonPresent);

	assert(supp.supportState("drivers/input/joystick/something.ko") ==
	       SupportedConf::Unsupported);
	assert(supp.supportState("drivers/mtd/spi-nor/spi-nor") == SupportedConf::Unspecified);
	assert(supp.supportState("drivers/mtd/something_else") == SupportedConf::Unsupported);
	assert(supp.supportState("drivers/usb/typec/altmodes/typec_displayport") ==
	       SupportedConf::Unsupported);
	assert(supp.supportState("fs/ocfs2/ocfs2_stackglue") == SupportedConf::KMPSupported);
	assert(supp.supportState("drivers/net/ethernet/8390/8390") == SupportedConf::BaseSupported);
	assert(supp.supportState("drivers/ata/ahci_imx") == SupportedConf::UnsupportedOptional);
	assert(supp.supportState("arch/powerpc/platforms/powernv/opal-prd") ==
	       SupportedConf::ExternallySupported);
}

std::string generatePatch(const std::string_view &ref, const std::string_view &ack,
			  const std::vector<std::string_view> &files)
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
		"Acked-by: Some User <" << ack << ">\n"
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

void testProcessPatch()
{
	static constexpr std::string_view patch("some.patch");
	static std::string ack("someone@suse.cz");
	static std::string file("file.c");
	static std::string file2("file2.c");

	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("stable-fixes", "noone@nowhere.com", {file}));
		assert(PA.m_HoH.size() == 0);
		assert(PA.m_HoHReal[ack][file] == 0);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("stable-fixes", ack, {file}));
		assert(PA.m_HoH[ack][file] == 1);
		assert(PA.m_HoHReal[ack][file] == 0);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("git-fixes", ack, {file}));
		assert(PA.m_HoH[ack][file] == 1);
		assert(PA.m_HoHReal[ack][file] == 0);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("bsc#123456", ack, {file}));
		assert(PA.m_HoH[ack][file] == 1);
		assert(PA.m_HoHReal[ack][file] == 1);
	}
	{
		PatchesAuthors PA;
		PA.processPatch(patch, generatePatch("bsc#123456", ack, {file, file2}));
		assert(PA.m_HoH[ack][file] == 1);
		assert(PA.m_HoHReal[ack][file] == 1);
		assert(PA.m_HoH[ack][file2] == 1);
		assert(PA.m_HoHReal[ack][file2] == 1);
	}
}

} // namespace

int main()
{
	testBranches();
	testSupportedConf();
	testProcessPatch();

	return 0;
}

