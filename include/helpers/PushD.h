// SPDX-License-Identifier: GPL-2.0-only

#ifndef PUSHD_H
#define PUSHD_H

namespace SlHelpers {

class PushD {
public:
	PushD() = delete;
	PushD(const std::filesystem::path &dir, std::error_code &ec) {
		origDir = std::filesystem::current_path(ec);
		if (!ec)
			std::filesystem::current_path(dir, ec);
	}
	~PushD() { std::filesystem::current_path(origDir); }
private:
	std::filesystem::path origDir;
};
}

#endif
