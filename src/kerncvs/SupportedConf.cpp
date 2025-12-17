// SPDX-License-Identifier: GPL-2.0-only

#include <fnmatch.h>
#include <iostream>
#include <vector>

#include "helpers/Color.h"
#include "helpers/String.h"
#include "kerncvs/SupportedConf.h"

using namespace SlKernCVS;

using Clr = SlHelpers::Color;

void SupportedConf::parseLine(std::string_view line) noexcept
{
	auto vec = SlHelpers::String::splitSV(line, " \t", '#');
	if (vec.empty())
		return;

	auto supp = SupportState::Unspecified;
	if (vec.size() >= 2) {
		const auto &suppFlag = vec[0];
		switch (suppFlag[0]) {
		case '+':
			if (suppFlag.ends_with("-kmp"))
				supp = SupportState::KMPSupported;
			else if (suppFlag == "+external")
				supp = SupportState::ExternallySupported;
			else if (suppFlag == "+base")
				supp = SupportState::BaseSupported;
			else
				supp = SupportState::Supported;
			break;
		case '-':
			if (suppFlag == "-!optional")
				supp = SupportState::UnsupportedOptional;
			else
				supp = SupportState::Unsupported;
			break;
		default:
			Clr(std::cerr, Clr::RED) << __func__ << ": bad vec from: " << line;
			return;
		}
	}

	auto module = vec.back();
	if (module.ends_with(".ko"))
		module.remove_suffix(3);
	entries.emplace_back(module, supp);
}

SupportedConf::SupportedConf(std::string_view conf)
{
	SlHelpers::GetLine gl(conf);
	while (auto line = gl.get())
		parseLine(*line);
}

SupportedConf::SupportState SupportedConf::supportState(const std::string &module) const
{
	for (const auto &e : entries)
		if (!::fnmatch(e.first.c_str(), module.c_str(), FNM_NOESCAPE | FNM_PERIOD))
			return e.second;

	return SupportState::NonPresent;

}
