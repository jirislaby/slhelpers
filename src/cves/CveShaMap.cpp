// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "helpers/Exception.h"
#include "helpers/Misc.h"
#include "helpers/String.h"

#include "cves/CVE.h"
#include "cves/CveShaMap.h"

using RunEx = SlHelpers::RuntimeException;
using SlHelpers::raise;

using namespace SlCVEs;

CveShaMap::CveShaMap(const std::filesystem::path &vsource, ShaSize shaSize,
		     const std::string &branch, unsigned year, bool rejected)
{
	std::filesystem::path source = vsource;

	if (source.empty()) {
		auto envSource = SlHelpers::Env::get<std::filesystem::path>("VULNS_GIT");
		if (!envSource)
			RunEx("vsource is empty and VULNS_GIT not set!").raise();
		source = *envSource;
	}

	const auto vulns_repo = SlGit::Repo::open(source);
	if (!vulns_repo)
		RunEx("Failed to open vulns repo at ") << source << ": " <<
			SlGit::Repo::lastError() << raise;

	const auto commit = vulns_repo->commitRevparseSingle(branch);
	if (!commit)
		RunEx("Failed to find branch ") << branch << " in vulns repo at " <<
			source << ": " << SlGit::Repo::lastError() << raise;

	std::string cve_prefix = rejected ? "cve/rejected/" : "cve/published/";
	if (year)
		cve_prefix += std::to_string(year) + '/';
	const auto subTree = commit->tree()->treeEntryByPath(cve_prefix);
	if (!subTree)
		RunEx("Failed to find tree ") << cve_prefix << " in vulns repo at " <<
			source << ": " << SlGit::Repo::lastError() << raise;

	const bool isShort = shaSize == ShaSize::Short;

	vulns_repo->treeLookup(*subTree)->walk([&vulns_repo, &isShort, this]
					       (const std::string &,
					       const SlGit::TreeEntry &entry) -> int {
		if (entry.type() != GIT_OBJECT_BLOB)
			return 0;
		const std::string file = entry.name();
		if (!file.ends_with(".sha1"))
			return 0;

		auto cve_number = CVE::getCVENumber(file);
		if (!cve_number) {
			std::cerr << file << " doesn't seem to be a cve_number.sha1!\n";
			return 0;
		}
		std::istringstream iss(vulns_repo->blobLookup(entry)->content());
		std::string sha_hash;
		while (iss >> sha_hash) {
			if (!SlHelpers::String::isHex(sha_hash) || sha_hash.size() != 40) {
				std::cerr << '"' << sha_hash <<
					     "\" doesn't seem to be a commit hash! (from a file \"" <<
					     file << "\")\n";
				continue;
			}
			if (isShort)
				m_shaCveMap.emplace(sha_hash.substr(0, 12), *cve_number);
			else {
				m_cveShaMap.emplace(*cve_number, sha_hash);
				m_shaCveMap.emplace(std::move(sha_hash), *cve_number);
			}
		}
		return 0;
	});
}
