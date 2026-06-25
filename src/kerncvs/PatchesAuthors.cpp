// SPDX-License-Identifier: GPL-2.0-only

#include <iomanip>
#include <iostream>
#include <set>
#include <string_view>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "helpers/String.h"
#include "helpers/SUSE.h"
#include "kerncvs/PatchesAuthors.h"

using namespace SlKernCVS;

constexpr std::string_view PatchesAuthors::parseEmail(std::string_view line, std::size_t atSignPos)
{
	auto start = line.find_last_of(" \t", atSignPos);
	if (start == std::string_view::npos)
		start = 0;
	else
		start++;

	auto end = line.find_first_of(" \t", atSignPos);
	if (end == std::string_view::npos)
		end = line.size();

	line = line.substr(start, end - start);

	if (line.starts_with('<') && line.ends_with('>'))
		line = line.substr(1, line.size() - 2);

	return line;
}

constexpr std::optional<std::string_view> PatchesAuthors::isInterestingLine(std::string_view line)
{
	line = SlHelpers::String::trim(line);

	constexpr const std::string_view interestingPrefixes[] = {
		"From:",
		"Cc:",
		"Co-developed-by:",
		"Acked:",
		"Acked-by:",
		"Modified-by:",
		"Reviewed-by:",
		"Reviewed-and-tested-by:",
		"Signed-off-by:"
	};

	for (const auto &prefix: interestingPrefixes)
		if (SlHelpers::String::iStartsWith(line, prefix)) {
			line = SlHelpers::String::trim(line.substr(prefix.size()));

			for (const auto &email: SlHelpers::SUSE::suseDomains) {
				auto pos = SlHelpers::String::iFind(line, email);
				if (pos != std::string_view::npos)
					return parseEmail(line, pos);
			}

			break;
		}

	return std::nullopt;
}

constexpr bool PatchesAuthors::isGitFixes(std::string_view line)
{
	auto pos = SlHelpers::String::iFind(line, "stable-");
	if (pos != std::string_view::npos && pos + 7 < line.size() &&
	    std::isdigit(static_cast<unsigned char>(line[pos + 7])))
		return true;

	constexpr const std::string_view gitFixes[] = {
		"git-fix",
		"git fix",
		"stable-fix",
		"stable fix",
		"bnc#1012628",
		"bsc#1012628",
		"bnc#1051510",
		"bsc#1051510",
		"bnc#1151927",
		"bsc#1151927",
		"bnc#1152489",
		"bsc#1152489",
	};

	for (const auto &gf : gitFixes)
		if (SlHelpers::String::iFind(line, gf) != std::string_view::npos)
			return true;

	return false;
}

constexpr bool PatchesAuthors::isReallyEmail(std::string_view line)
{
	if (line.starts_with('['))
		return false;
	if (line.ends_with(':'))
		return false;

	constexpr const std::string_view invalidStarts[] = {
		"Debugged-by:",
		"Evaluated-by:",
		"Improvements-by:",
		"Link:",
		"Message-ID:",
		"Patch-mainline:",
		"Reported-and-tested-by:",
		"Reported-by:",
		"Return-path:",
		"Suggested-by:",
		"Tested-by:",
	};

	for (const auto &invalidStart: invalidStarts)
		if (SlHelpers::String::iStartsWith(line, invalidStart))
			return false;


	constexpr const std::string_view invalidSubstrs[] = {
		"lore.kernel",
		"lkml.kernel",
		"patchwork.ozlabs",
		"thanks",
	};

	for (const auto &invalidSubstr: invalidSubstrs)
		if (SlHelpers::String::iFind(line, invalidSubstr) != std::string_view::npos)
			return false;

	return true;
}

constexpr bool PatchesAuthors::isValidRef(std::string_view ref)
{
	constexpr const std::string_view validRefs[] = {
		"bnc#",
		"boo#",
		"bsc#",
		"CVE-",
		"FATE#",
		"https://",
		"jsc#",
		"kabi",
		"ltc#",
		"poo#",
		"XSA-",
	};

	for (const auto &validRef : validRefs)
		if (SlHelpers::String::iFind(ref, validRef) != std::string_view::npos)
			return true;

	return false;
}

int PatchesAuthors::processPatch(const std::filesystem::path &file, const std::string &content)
{
	std::set<std::string> patchEmails;
	std::set<std::string> patchRefs;
	bool gitFixes = false;
	SlHelpers::GetLine gl(content);
	while (auto lineOpt = gl.get()) {
		auto line = *lineOpt;
		auto m = isInterestingLine(line);
		if (m) {
			patchEmails.emplace(*m);
			continue;
		}
		if (line.starts_with("---"))
			break;

		constexpr const std::string_view references("References:");
		if (SlHelpers::String::iStartsWith(line, references)) {
			line.remove_prefix(references.size());

			if (isGitFixes(line)) {
				gitFixes = true;
			} else if (dumpRefs) {
				for (const auto &ref: SlHelpers::String::splitSV(line, " \t,;"))
					patchRefs.emplace(ref);
			}
		}

		if (reportUnhandled && line.find("@suse.") != std::string::npos &&
		    isReallyEmail(line))
			std::cerr << file << ": unhandled e-mail in '" << line << "'\n";
	}

	for (const auto &ref : patchRefs)
		for (const auto &email : patchEmails)
			if (!isValidRef(ref))
				m_HoHRefs[email][ref]++;

	while (auto lineOpt = gl.get()) {
		auto line = *lineOpt;
		static constexpr const std::string_view prefix("+++ b/");
		if (!line.starts_with(prefix))
			continue;
		if (!line.ends_with(".c") && !line.ends_with(".h"))
			continue;

		auto cfile = std::string(line.substr(prefix.length()));
		if (cfile.starts_with("/dev"))
			std::cerr << __func__ << ": " << file << ": " << cfile << '\n';
		for (const auto &email : patchEmails) {
			m_HoH[email][cfile]++;
			if (gitFixes)
				m_HoHReal[email].try_emplace(cfile, 0);
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
	if (!patchesSuseTree->walk([this](const std::string &root,
					 const SlGit::TreeEntry &entry) -> int {
		auto blob = repo->blobLookup(entry);
		if (!blob)
			return -1000;

		return processPatch(root + entry.name(), blob->content());
	    }))
		return false;

	for (const auto &pair : m_HoHRefs)
		for (const auto &refPair : pair.second)
			if (refPair.second) {
				std::cout << std::setw(30) << pair.first <<
					     std::setw(40) << std::quoted(refPair.first) <<
					     std::setw(5) << refPair.second << '\n';
			}

	for (const auto &pair : m_HoH) {
		const auto &email = pair.first;
		const auto &realMap = m_HoHReal.at(email);
		if (!insertUser(email))
			return false;

		for (const auto &pairSrc : pair.second) {
			std::filesystem::path path(pairSrc.first);

			if (!insertUFMap(email, std::move(path), pairSrc.second,
					 realMap.at(pairSrc.first)))
				return false;
		}
	}

	return true;
}
