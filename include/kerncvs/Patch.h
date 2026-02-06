// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <istream>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "../helpers/LastError.h"

namespace SlKernCVS {

/**
 * @brief Parses a patch into header and files patched.
 */
class Patch {
public:
	/// @brief Type for a patch header
	using Header = std::vector<std::string>;
	/// @brief Type for patched paths
	using Paths = std::set<std::filesystem::path>;

	Patch() = delete;

	/// @brief Create a new Patch from file at \p path
	static std::optional<Patch> create(const std::filesystem::path &path);
	/// @brief Create a new Patch from stream \p is
	static std::optional<Patch> create(std::istream &is);

	/// @brief Get header of this Patch
	const auto &header() const noexcept { return m_header; }
	/// @brief Get paths this Patch touches
	const auto &paths() const noexcept { return m_paths; }

	/// @brief Return the last error string if any
	static auto lastError() noexcept { return m_lastError.lastError(); }
private:
	Patch(Header &&header, Paths &&paths) : m_header(std::move(header)),
		m_paths(std::move(paths)) {}

	Header m_header;
	Paths m_paths;

	using LastError = SlHelpers::LastErrorStream<>;
	static thread_local LastError m_lastError;
};

}
