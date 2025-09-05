// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

namespace SlGit {

static struct LibIniter {
	LibIniter()	{ git_libgit2_init(); }
	~LibIniter()	{ git_libgit2_shutdown(); }
} LI;

}
