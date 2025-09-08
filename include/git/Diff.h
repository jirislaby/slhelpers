// SPDX-License-Identifier: GPL-2.0-only

#ifndef GIT_DIFF_H
#define GIT_DIFF_H

#include <functional>

#include <git2.h>

#include "../helpers/Unique.h"

namespace SlGit {

class Repo;

class Diff {
	using GitTy = git_diff;
	using Holder = SlHelpers::UniqueHolder<GitTy>;

	friend class Repo;
public:
	using PrintCB = std::function<int(const git_diff_delta &delta, const git_diff_hunk *hunk,
		const git_diff_line &line)>;
	class ForEachCB {
	public:
		virtual int file(const git_diff_delta *delta, float progress) const = 0;
		virtual int binary(const git_diff_delta *delta,
				   const git_diff_binary *binary) const = 0;
		virtual int hunk(const git_diff_delta *delta, const git_diff_hunk *hunk) const = 0;
		virtual int line(const git_diff_delta *delta, const git_diff_hunk *hunk,
				 const git_diff_line *line) const = 0;
	};

	Diff() = delete;

	size_t numDeltas() const noexcept { return git_diff_num_deltas(diff()); }
	size_t numDeltas(const git_delta_t &type) const noexcept {
		return git_diff_num_deltas_of_type(diff(), type);
	}
	const git_diff_delta *getDelta(const size_t &idx) const noexcept {
		return git_diff_get_delta(diff(), idx);
	}

	bool isSortedICase() const noexcept { return git_diff_is_sorted_icase(diff()); }

	int forEach(const ForEachCB &forEachCB) const;
	int print(const git_diff_format_t &format, const PrintCB &printCB) const;

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

#endif
