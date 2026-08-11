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

void Maintainers::readSUSEStanza(std::ifstream &file, Stanza &st,
				 const Stanza::TranslateEmail &translateEmail)
{
	for (std::string line; getline(file, line);) {
		auto lineSV = SlHelpers::String::trim(std::string_view(line));
		if (lineSV.empty())
			break;
		if (lineSV.size() < 3)
			continue;
		auto colon = lineSV.find(':');
		if (colon == std::string_view::npos)
			continue;
		auto lhs = lineSV.substr(0, colon);
		auto rhs = SlHelpers::String::trim(lineSV.substr(colon + 1));
		if (rhs.empty()) {
			std::cerr << "Bad MAINTAINERS entry: " << lineSV << '\n';
			continue;
		}
		if (lhs == "COMMENT")
			st.add_comment(std::string(rhs));
		else if (lhs == "F")
			st.add_pattern(std::string(rhs));
		else if (lhs == "M")
			st.add_maintainer_and_store(lineSV, m_suse_users, translateEmail);
	}
}

void Maintainers::loadSUSE(const std::filesystem::path &filename,
			   const Stanza::TranslateEmail &translateEmail)
{
	std::ifstream file{filename};

	if (!file.is_open())
		RunEx("Unable to open MAINTAINERS file: ") << filename << ": " <<
			strerror(errno) << raise;

	for (std::string line; getline(file, line);) {
		line = SlHelpers::String::trim(line);
		if (line.empty())
			continue;
		Stanza st(std::move(line));
		readSUSEStanza(file, st, translateEmail);

		if (!st.empty())
			m_maintainers.emplace_back(std::move(st));
	}

	if (m_maintainers.empty())
		RunEx() << filename << " appears to be empty" << raise;
}

void Maintainers::skipIntroUpstream(SlHelpers::GetLine &gl)
{
	while (auto lineOpt = gl.get())
		if (lineOpt->starts_with("Maintainers List"))
			break;

	while (auto lineOpt = gl.get()) {
		if (lineOpt->empty())
			continue;
		if (lineOpt->starts_with("----"))
			continue;
		if (lineOpt->starts_with(".. "))
			continue;
		if (lineOpt->starts_with("   "))
			continue;

		return;
	}

	RunEx("Upstream MAINTAINERS has no \"Maintainers List\"?").raise();
}

void Maintainers::readUpstreamStanza(SlHelpers::GetLine &gl, Stanza &st,
				     const Stanza::TranslateEmail &translateEmail)
{
	while (auto lineOpt = gl.get()) {
		auto line = *lineOpt;
		if (line.size() < 3)
			break;
		if (line[1] != ':')
			continue;

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
			auto fpattern = SlHelpers::String::trim(line.substr(2));
			if (fpattern.empty())
				std::cerr << "Bad upstream MAINTAINERS entry (pattern): " <<
					line << '\n';
			else
				st.add_pattern(std::string(fpattern));
			break;
		}
	}
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

	SlHelpers::GetLine gl(*maintOpt);

	skipIntroUpstream(gl);

	while (auto lineOpt = gl.get()) {
		auto line = *lineOpt;
		if (line == "THE REST")
			break;
		if (line.size() < 3)
			continue;

		Stanza st(std::string("Upstream: ").append(line));
		readUpstreamStanza(gl, st, translateEmail);

		if (!st.empty())
			m_upstream_maintainers.emplace_back(std::move(st));
	}

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
