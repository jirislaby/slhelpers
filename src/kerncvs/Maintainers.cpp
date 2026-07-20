// SPDX-License-Identifier: GPL-2.0-only

#include <cstring>
#include <fstream>

#include "git/Repo.h"
#include "helpers/Exception.h"
#include "helpers/String.h"
#include "kerncvs/Maintainers.h"

using RunEx = SlHelpers::RuntimeException;
using SlHelpers::raise;

using namespace SlKernCVS;

void Maintainers::loadSUSE(const std::filesystem::path &filename,
			   const Stanza::TranslateEmail &translateEmail)
{
	std::ifstream file{filename};

	if (!file.is_open())
		RunEx("Unable to open MAINTAINERS file: ") << filename << ": " <<
			strerror(errno) << raise;

	Stanza st;
	for (std::string line; getline(file, line);) {
		const auto tmp = SlHelpers::String::trim(line);
		if (tmp.size() < 2)
			continue;
		if (tmp[1] == ':') {
			if (tmp[0] == 'M')
				st.add_maintainer_and_store(tmp, m_suse_users, translateEmail);
			else if (tmp[0] == 'F') {
				const auto fpattern = SlHelpers::String::trim(tmp.substr(2));
				if (fpattern.empty())
					std::cerr <<  "MAINTAINERS entry: " << tmp << '\n';
				else
					st.add_pattern(fpattern);
			}
		} else {
			if (!st.empty())
				m_maintainers.push_back(std::move(st));
			st.new_entry(std::move(tmp));
		}
	}
	if (!st.empty())
		m_maintainers.push_back(std::move(st));

	if (m_maintainers.empty())
		RunEx() << filename << " appears to be empty" << raise;
}

void Maintainers::loadUpstream(const std::filesystem::path &lsource, const std::string &origin,
			       const Stanza::TranslateEmail &translateEmail)
{
	auto linux_repo = SlGit::Repo::open(lsource);
	if (!linux_repo)
		RunEx("Unable to open linux.git at ") << lsource << ": " <<
			SlGit::Repo::lastError() << raise;

	auto maintOpt = linux_repo->catFile(origin + "/master", "MAINTAINERS");
	if (!maintOpt)
		RunEx("Unable to load linux.git tree for ") << origin << "/master: " <<
			     SlGit::Repo::lastError() << raise;

	Stanza st;
	bool skip = true;
	SlHelpers::GetLine gl(*maintOpt);
	while (auto lineOpt = gl.get()) {
		auto line = *lineOpt;
		if (skip) {
			if (line.starts_with("Maintainers List"))
				skip = false;
			continue;
		}
		if (line == "THE REST")
			break;
		if (line.size() < 3 || std::strchr("\t .-", line[1]))
			continue;
		if (line[1] == ':')
			switch(line[0]) {
			case 'L': // TODO?
			case 'S':
			case 'W':
			case 'Q':
			case 'B':
			case 'C':
			case 'P':
			case 'T':
			case 'X':
			case 'N': // TODO, huh?
			case 'K':
				break;
			case 'M':
			case 'R':
				st.add_maintainer_if(line, m_suse_users, translateEmail);
				break;
			case 'F':
				std::string fpattern(SlHelpers::String::trim(line.substr(2)));
				if (fpattern.empty())
					std::cerr << "Upstream MAINTAINERS entry: " << line << '\n';
				else
					st.add_pattern(std::move(fpattern));
				break;
			}
		else {
			if (!st.empty())
				m_upstream_maintainers.push_back(std::move(st));
			st.new_entry(std::string("Upstream: ").append(line));
		}
	}
	if (!st.empty())
		m_upstream_maintainers.push_back(std::move(st));
	if (m_upstream_maintainers.empty())
		RunEx("Upstream MAINTAINERS appears to be empty").raise();
}

const Stanza *Maintainers::findBestMatchInMaintainers(const MaintainersType &sl,
						      const std::set<std::filesystem::path> &paths)
{
	const Stanza *ret = nullptr;
	unsigned best_weight = 0;
	for(const auto &s: sl) {
		unsigned weight = 0;
		for (const auto &path: paths)
			weight += s.match_path(path);
		if (weight > best_weight) {
			ret = &s;
			best_weight = weight;
		}
	}
	return ret;
}
