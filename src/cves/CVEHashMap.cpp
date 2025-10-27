// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Repo.h"
#include "git/Tree.h"
#include "helpers/String.h"

#include "cves/CVEHashMap.h"

using namespace SlCVEs;

bool CVEHashMap::load(const std::filesystem::path &vsource)
{
	if (vsource.empty())
		return false;

	auto vulns_repo = SlGit::Repo::open(vsource);
	if (!vulns_repo)
		return false;

	auto commit = vulns_repo->commitRevparseSingle(branch);
	if (!commit)
		return false;

	std::string cve_prefix = rejected ? "cve/rejected/" : "cve/published/";
	if (year)
		cve_prefix += std::to_string(year) + '/';
	const auto subTree = commit->tree()->treeEntryByPath(cve_prefix);
	if (!subTree)
		return false;

	const auto regex_cve_number = std::regex("CVE-[0-9][0-9][0-9][0-9]-[0-9]+", std::regex::optimize);

	vulns_repo->treeLookup(*subTree)->walk([&regex_cve_number, &vulns_repo, this](const std::string &,
					       const SlGit::TreeEntry &entry) -> int {
		if (entry.type() != GIT_OBJECT_BLOB)
			return 0;
		const std::string file = entry.name();
		if (!SlHelpers::String::endsWith(file, ".sha1"))
			return 0;

		std::smatch match;
		std::regex_search(file, match, regex_cve_number);
		std::string cve_number = match.str();
		if (cve_number.size() < 10) {
			std::cout << cve_number << " doesn't seem to be a cve number!\n";
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
			if (shaSize == ShaSize::Short)
				m_sha_hash_map.insert(std::make_pair(sha_hash.substr(0, 12), cve_number));
			else {
				m_cve_hash_multimap.insert(std::make_pair(cve_number, sha_hash));
				m_sha_hash_map.insert(std::make_pair(std::move(sha_hash), cve_number));
			}
		}
		return 0;
	});

	return true;
}
