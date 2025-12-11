// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "helpers/String.h"

#include "cves/CVE.h"
#include "cves/CVEHashMap.h"

using namespace SlCVEs;

std::optional<CVEHashMap> CVEHashMap::create(const std::filesystem::path &vsource,
					     ShaSize shaSize, const std::string &branch,
					     unsigned year, bool rejected)

{
	if (vsource.empty())
		return std::nullopt;

	const auto vulns_repo = SlGit::Repo::open(vsource);
	if (!vulns_repo)
		return std::nullopt;

	const auto commit = vulns_repo->commitRevparseSingle(branch);
	if (!commit)
		return std::nullopt;

	std::string cve_prefix = rejected ? "cve/rejected/" : "cve/published/";
	if (year)
		cve_prefix += std::to_string(year) + '/';
	const auto subTree = commit->tree()->treeEntryByPath(cve_prefix);
	if (!subTree)
		return std::nullopt;

	const bool isShort = shaSize == ShaSize::Short;
	CVEHashMapTy cveMap;
	SHAHashMapTy shaMap;

	vulns_repo->treeLookup(*subTree)->walk([&vulns_repo, &isShort, &shaMap, &cveMap]
					       (const std::string &,
					       const SlGit::TreeEntry &entry) -> int {
		if (entry.type() != GIT_OBJECT_BLOB)
			return 0;
		const std::string file = entry.name();
		if (!SlHelpers::String::endsWith(file, ".sha1"))
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
				shaMap.insert(std::make_pair(sha_hash.substr(0, 12), *cve_number));
			else {
				cveMap.insert(std::make_pair(*cve_number, sha_hash));
				shaMap.insert(std::make_pair(std::move(sha_hash), *cve_number));
			}
		}
		return 0;
	});

	return CVEHashMap(std::move(cveMap), std::move(shaMap));
}
