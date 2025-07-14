#include <iostream>
#include <string>

#include "git/Git.h"
#include "kerncvs/CollectConfigs.h"

int main()
{
	auto repo = SlGit::Repo::open("/home/xslaby/my-repos/file2config/build-file2config-Desktop-debug/fill-db/kernel-source/");
	if (!repo)
		return EXIT_FAILURE;

	auto commit = repo->commitRevparseSingle("refs/remotes/origin/SL-16.0");
	if (!commit) {
		std::cerr << "cannot find SL-16.0: " << git_error_last()->message << '\n';
		return EXIT_FAILURE;
	}

	SlKernCVS::CollectConfigs CC{*repo,
		[](const std::string &arch, const std::string &flavor) -> int {
			std::cout << "arch=" << arch << " flavor=" << flavor << '\n';
			return 0;
		}, [](const std::string &, const std::string &,
		      const std::string &conf,
		      const SlKernCVS::CollectConfigs::ConfigValue &value) -> int {
			std::cout << '\t' << conf << '=' << value << '\n';
			return 0;
		}};

	if (CC.collectConfigs(*commit))
		return EXIT_FAILURE;

	return 0;
}

