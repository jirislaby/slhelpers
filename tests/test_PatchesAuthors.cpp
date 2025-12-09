// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "kerncvs/PatchesAuthors.h"

using namespace SlKernCVS;

namespace {

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

}

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

}

int main()
{
	testProcessPatch();
	return 0;
}

