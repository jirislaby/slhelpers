// SPDX-License-Identifier: GPL-2.0-only

#include <fstream>

#include "kerncvs/Patch.h"

using namespace SlKernCVS;

thread_local Patch::LastError Patch::m_lastError;

std::optional<Patch> Patch::create(const std::filesystem::path &path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		m_lastError.reset() << "Unable to open diff file: " << path;
		return std::nullopt;
	}

	return create(file);
}

std::optional<Patch> Patch::create(std::istream &is)
{
	Header header;
	Paths paths;
	bool inHeader = true;

	for (std::string line; std::getline(is, line); ) {
		line.erase(0, line.find_first_not_of(" \t"));
		if (inHeader) {
			if (line.starts_with("---")) {
				inHeader = false;
				continue;
			}
			header.emplace_back(std::move(line));
			continue;
		}

		if (line.starts_with("--- a/") || line.starts_with("+++ b/"))
			paths.emplace(line.substr(6));
	}

	return Patch(std::move(header), std::move(paths));
}
