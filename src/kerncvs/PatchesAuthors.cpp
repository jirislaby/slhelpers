// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <set>

#include "helpers/String.h"
#include "kerncvs/PatchesAuthors.h"
#include "git/Git.h"

using namespace SlKernCVS;

const std::regex PatchesAuthors::REInteresting("^\\s*(?:From|Cc|Co-developed-by|Acked|Acked-by|Modified-by|Reviewed-by|Reviewed-and-tested-by|Signed-off-by):.*[\\s<]([a-z0-9_.-]+\\@suse\\.[a-z]+)",
	      std::regex_constants::icase);
const std::regex PatchesAuthors::REFalse("(?:lore|lkml)\\.kernel|patchwork\\.ozlabs|^\\[|^(?:Debugged-by|Evaluated-by|Improvements-by|Link|Message-ID|Patch-mainline|Reported-and-tested-by|Reported-by|Return-path|Suggested-by|Tested-by):|thanks|:$",
	      std::regex_constants::icase);
const std::regex PatchesAuthors::REGitFixes("^References:.*(?:(?:git|stable)[- ]fixes|stable-\\d|b[ns]c[#](?:1012628|1051510|1151927|1152489))",
	   std::regex_constants::icase);
const std::regex PatchesAuthors::REInvalRef("FATE#|CVE-|jsc#|XSA-", std::regex_constants::icase);

int PatchesAuthors::processPatch(const std::filesystem::path &file, const std::string &content)
{
	std::set<std::string> patchEmails;
	std::set<std::string> patchRefs;
	bool gitFixes = false;
	std::istringstream iss(content);
	std::string line;
	std::smatch m;

	while (std::getline(iss, line)) {
		if (std::regex_search(line, m, REInteresting)) {
			patchEmails.insert(m[1]);
			continue;
		}
		if (SlHelpers::String::startsWith(line, "---"))
			break;
		if (std::regex_search(line, REGitFixes)) {
			gitFixes = true;
		} else if (dumpRefs) {
			static const std::string references { "References:" };
			if (SlHelpers::String::startsWith(line, references))
				for (const auto &ref: SlHelpers::String::split(line.substr(references.size()),
									" \t,;"))
					patchRefs.insert(ref);
		}

		if (reportUnhandled && line.find("@suse.") != std::string::npos &&
				!std::regex_search(line, REFalse))
			std::cerr << file << ": unhandled e-mail in '" << line << "'\n";
	}

	for (const auto &ref : patchRefs)
		for (const auto &email : patchEmails)
			if (!std::regex_search(ref, REInvalRef))
				m_HoHRefs[email][ref]++;

	while (std::getline(iss, line)) {
		static const std::string prefix { "+++ b/" };
		if (!SlHelpers::String::startsWith(line, prefix))
			continue;
		if (!SlHelpers::String::endsWith(line, ".c") &&
				!SlHelpers::String::endsWith(line, ".h"))
			continue;

		auto cfile = line.substr(prefix.length());
		if (SlHelpers::String::startsWith(cfile, "/dev"))
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

	auto patchesSuseTree = repo.treeLookup(*patchesSuseTreeEntry);
	if (!patchesSuseTree)
		return false;
	auto ret = patchesSuseTree->walk([this](const std::string &root,
					 const SlGit::TreeEntry &entry) -> int {
		auto blob = repo.blobLookup(entry);
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
