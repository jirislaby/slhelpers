// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <functional>

#include <git2.h>
#include <optional>

#include "../helpers/Unique.h"

#include "Buf.h"

namespace SlGit {

class Repo;

class Diff {
	using GitTy = git_diff;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	using PrintCB = std::function<int(const git_diff_delta &delta, const git_diff_hunk *hunk,
		const git_diff_line &line)>;

	struct ForEachCB {
		const std::function<int(const git_diff_delta &delta,
					float progress)> file = nullptr;
		const std::function<int(const git_diff_delta &,
					const git_diff_binary &)> binary = nullptr;
		const std::function<int(const git_diff_delta &,
					const git_diff_hunk &)> hunk = nullptr;
		const std::function<int(const git_diff_delta &, const git_diff_hunk &,
						const git_diff_line &)> line = nullptr;
	};

	Diff() = delete;

	size_t numDeltas() const noexcept { return git_diff_num_deltas(diff()); }
	size_t numDeltas(const git_delta_t &type) const noexcept {
		return git_diff_num_deltas_of_type(diff(), type);
	}
	const git_diff_delta *getDelta(const size_t &idx) const noexcept {
		return git_diff_get_delta(diff(), idx);
	}

	bool findSimilar(const git_diff_find_options *options = nullptr) const noexcept {
		return !git_diff_find_similar(diff(), options);
	}

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
	int print(const git_diff_format_t &format, const PrintCB &printCB) const;

	std::optional<Buf> toBuf(git_diff_format_t format) const noexcept;

	GitTy *diff() const noexcept { return m_diff.get(); }
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
