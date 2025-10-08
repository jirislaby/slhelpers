// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Diff.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_diff>::operator()(git_diff *idx) const
{
	git_diff_free(idx);
}

int Diff::forEach(const ForEachCB &forEachCB) const
{
	return git_diff_foreach(diff(), fileCB, binaryCB, hunkCB, lineCB,
			const_cast<void *>(static_cast<const void *>(&forEachCB)));
}

int Diff::print(const git_diff_format_t &format, const PrintCB &printCB) const
{
	return git_diff_print(diff(), format, Diff::printCB,
			      const_cast<void *>(static_cast<const void *>(&printCB)));
}

int Diff::fileCB(const git_diff_delta *delta, float progress, void *payload)
{
	const auto &cb = *static_cast<const ForEachCB *>(payload);

	return cb.file(delta, progress);
}

int Diff::binaryCB(const git_diff_delta *delta, const git_diff_binary *binary, void *payload)
{
	const auto &cb = *static_cast<const ForEachCB *>(payload);

	return cb.binary(delta, binary);
}

int Diff::hunkCB(const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload)
{
	const auto &cb = *static_cast<const ForEachCB *>(payload);

	return cb.hunk(delta, hunk);
}

int Diff::lineCB(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line,
		 void *payload)
{
	const auto &cb = *static_cast<const ForEachCB *>(payload);

	return cb.line(delta, hunk, line);
}

int Diff::printCB(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line,
		  void *payload)
{
	const auto &cb = *static_cast<const PrintCB *>(payload);

	return cb(*delta, hunk, *line);
}
