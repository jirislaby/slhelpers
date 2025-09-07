// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Blob.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_blob>::operator()(git_blob *blob) const
{
	git_blob_free(blob);
}

Blob::Blob(git_blob *blob) : m_blob(blob)
{
}
