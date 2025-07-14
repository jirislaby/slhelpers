#include <iostream>
#include <string>

#include "git/Commit.h"
#include "git/Repo.h"

int main()
{
	auto repo = SlGit::Repo::clone("/tmp/l",
				       "https://github.com/rpm-software-management/rpm");
	if (!repo) {
		std::cerr << "cannot clone: " << git_error_last()->message << '\n';
		return EXIT_FAILURE;
	}

	auto commit = repo->commitRevparseSingle("refs/remotes/origin/master");
	if (!commit) {
		std::cerr << "cannot find master: " << git_error_last()->message << '\n';
		return EXIT_FAILURE;
	}

	return 0;
}
