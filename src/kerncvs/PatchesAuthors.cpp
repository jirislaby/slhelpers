// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <mutex>
#include <set>

#include "helpers/String.h"
#include "kerncvs/PatchesAuthors.h"
#include "git/Git.h"
#include "pcre2/PCRE2.h"

using namespace SlKernCVS;


int PatchesAuthors::processPatch(const std::filesystem::path &file, const std::string &content)
{
	static PCRE2::PCRE2 REInteresting;
	static PCRE2::PCRE2 REFalse;
	static PCRE2::PCRE2 REGitFixes;
	static PCRE2::PCRE2 REInvalRef;
	static std::once_flag flag;

	std::call_once(flag, [](){
		REInteresting.compile("^\\s*(?:From|Cc|Co-developed-by|Acked|Acked-by|Modified-by|Reviewed-by|Reviewed-and-tested-by|Signed-off-by):.*[\\s<]([a-z0-9_.-]+\\@suse\\.[a-z]+)",
				      PCRE2_CASELESS);
		REFalse.compile("(?:lore|lkml)\\.kernel|patchwork\\.ozlabs|^\\[|^(?:Debugged-by|Evaluated-by|Improvements-by|Link|Message-ID|Patch-mainline|Reported-and-tested-by|Reported-by|Return-path|Suggested-by|Tested-by):|thanks|:$",
				PCRE2_CASELESS);
		REGitFixes.compile("^References:.*(?:(?:git|stable)[- ]fixes|stable-\\d|b[ns]c#(?:1012628|1051510|1151927|1152489))",
				   PCRE2_CASELESS);
		REInvalRef.compile("FATE#|CVE-|jsc#|XSA-", PCRE2_CASELESS);
	});

	std::set<std::string> patchEmails;
	std::set<std::string> patchRefs;
	bool gitFixes = false;
	std::istringstream iss(content);
	std::string line;

	while (std::getline(iss, line)) {
		auto m = REInteresting.match(line);
		if (m > 1) {
			patchEmails.emplace(REInteresting.matchByIdx(line, 1));
			continue;
		}
		if (line.starts_with("---"))
			break;
		if (REGitFixes.match(line) > 0) {
			gitFixes = true;
		} else if (dumpRefs) {
			static const std::string references { "References:" };
			if (line.starts_with(references))
				for (const auto &ref: SlHelpers::String::split(line.substr(references.size()),
									" \t,;"))
					patchRefs.insert(ref);
		}

		if (reportUnhandled && line.find("@suse.") != std::string::npos &&
				REFalse.match(line) == PCRE2_ERROR_NOMATCH)
			std::cerr << file << ": unhandled e-mail in '" << line << "'\n";
	}

	for (const auto &ref : patchRefs)
		for (const auto &email : patchEmails)
			if (REInvalRef.match(ref) == PCRE2_ERROR_NOMATCH)
				m_HoHRefs[email][ref]++;

	while (std::getline(iss, line)) {
		static const std::string prefix { "+++ b/" };
		if (!line.starts_with(prefix))
			continue;
		if (!line.ends_with(".c") && !line.ends_with(".h"))
			continue;

		auto cfile = line.substr(prefix.length());
		if (cfile.starts_with("/dev"))
			std::cerr << __func__ << ": " << file << ": " << cfile << '\n';
		for (const auto &email : patchEmails) {
			m_HoH[email][cfile]++;
			if (gitFixes)
				m_HoHReal[email][cfile]; // add so it exists
			else
				m_HoHReal[email][cfile]++;
		}
	}

	return 0;
}

bool PatchesAuthors::processAuthors(const SlGit::Commit &commit, const InsertUser &insertUser,
				   const InsertUFMap &insertUFMap)
{
	auto tree = *commit.tree();

	auto patchesSuseTreeEntry = tree.treeEntryByPath("patches.suse/");
	if (!patchesSuseTreeEntry)
		return false;
	if (patchesSuseTreeEntry->type() != GIT_OBJECT_TREE)
		return false;

	auto patchesSuseTree = repo->treeLookup(*patchesSuseTreeEntry);
	if (!patchesSuseTree)
		return false;
	auto ret = patchesSuseTree->walk([this](const std::string &root,
					 const SlGit::TreeEntry &entry) -> int {
		auto blob = repo->blobLookup(entry);
		if (!blob)
			return -1000;

		return processPatch(root + entry.name(), blob->content());
	});
	if (ret)
		return false;

	for (const auto &pair : m_HoHRefs)
		for (const auto &refPair : pair.second)
			if (refPair.second > 100) {
				std::cout << std::setw(30) << pair.first <<
					     std::setw(40) << refPair.first <<
					     std::setw(5) << refPair.second << '\n';
			}

	for (const auto &pair : m_HoH) {
		const auto &email = pair.first;
		const auto &realMap = m_HoHReal.at(email);
		if (!insertUser(email))
			return false;

		for (const auto &pairSrc : pair.second) {
			std::filesystem::path path(pairSrc.first);

			if (!insertUFMap(email, path, pairSrc.second, realMap.at(pairSrc.first)))
				return false;
		}
	}

	return true;
}
