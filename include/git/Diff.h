// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <functional>

#include <git2.h>
#include <optional>

#include "../helpers/Unique.h"

#include "Buf.h"

namespace SlGit {

class Repo;

/**
 * @brief Diff is a representation of a git diff
 */
class Diff {
	using GitTy = git_diff;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	/// @brief Callback for print()
	using PrintCB = std::function<int(const git_diff_delta &delta, const git_diff_hunk *hunk,
		const git_diff_line &line)>;

	/// @brief Callbacks for forEach()
	struct ForEachCB {
		/// @brief Callback for each file (\c old_file and \c new_file in \p delta)
		const std::function<int(const git_diff_delta &delta,
					float progress)> file = nullptr;
		/// @brief Callback for each binary \p bin
		const std::function<int(const git_diff_delta &delta,
					const git_diff_binary &bin)> binary = nullptr;
		/// @brief Callback for each \p hunk
		const std::function<int(const git_diff_delta &delta,
					const git_diff_hunk &hunk)> hunk = nullptr;
		/// @brief Callback for each \p line
		const std::function<int(const git_diff_delta &delta, const git_diff_hunk &hunk,
					const git_diff_line &line)> line = nullptr;
	};

	Diff() = delete;

	/// @brief Create a new Diff from \p buffer
	static std::optional<Diff> createFromBuffer(std::string_view buffer) noexcept;

	/// @brief Get count of deltas in this Diff
	size_t numDeltas() const noexcept { return git_diff_num_deltas(diff()); }
	/// @brief Get count of deltas with the \p type (\c GIT_DELTA_ADDED, ...)
	size_t numDeltas(const git_delta_t &type) const noexcept {
		return git_diff_num_deltas_of_type(diff(), type);
	}
	/// @brief Get a delta on the \p idx-th position
	const git_diff_delta *getDelta(size_t idx) const noexcept {
		return git_diff_get_delta(diff(), idx);
	}

	/**
	 * @brief Find similar files in the Diff (in-place)
	 * @param options Options to use (like rename threshold)
	 * @return True on success.
	 * \code{.sh} git diff -M \endcode
	 */
	bool findSimilar(const git_diff_find_options *options = nullptr) const noexcept {
		return !git_diff_find_similar(diff(), options);
	}

	/// @brief Return true if ignore-case-sorted
	bool isSortedICase() const noexcept { return git_diff_is_sorted_icase(diff()); }

	/**
	 * @brief Invoke \p forEachCB callbacks for each file, hunk, ... in the diff
	 * @param forEachCB Callbacks
	 * @return 0 on success, non-zero callback return value, or error code.
	 */
	int forEach(const ForEachCB &forEachCB) const;
	/**
	 * @brief Invoke \p forEachCB callbacks for each file, hunk, ... in the diff
	 * @param format \c GIT_DIFF_FORMAT_PATCH, \c GIT_DIFF_FORMAT_PATCH_HEADER, ...
	 * @param printCB Callback
	 * @return 0 on success, non-zero callback return value, or error code.
	 */
	int print(git_diff_format_t format, const PrintCB &printCB) const;

	/// @brief Convert to Buf
	std::optional<Buf> toBuf(git_diff_format_t format) const noexcept;

	/// @brief Get the stored pointer to libgit2's git_diff
	GitTy *diff() const noexcept { return m_diff.get(); }
	/// @brief Alias for diff() -- implicit conversion
	operator GitTy *() const noexcept { return diff(); }
private:
	explicit Diff(GitTy *diff) noexcept : m_diff(diff) { }

	static int fileCB(const git_diff_delta *delta, float progress, void *payload);
	static int binaryCB(const git_diff_delta *delta, const git_diff_binary *binary,
			    void *payload);
	static int hunkCB(const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload);
	static int lineCB(const git_diff_delta *delta, const git_diff_hunk *hunk,
			  const git_diff_line *line, void *payload);
	static int printCB(const git_diff_delta *delta, const git_diff_hunk *hunk,
			   const git_diff_line *line, void *payload);

	Holder m_diff;
};

}
