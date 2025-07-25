#include <cassert>
#include <iostream>
#include <fstream>

#include "git/Git.h"

int main()
{
	SlGit::Repo repo;

	std::string s = std::filesystem::temp_directory_path() / "testgitdir.XXXXXX";
	assert(mkdtemp(s.data()));
	std::filesystem::path gitDir{s};
	std::cout << gitDir << '\n';

	SlGit::Signature me;
	assert(!me.now("Jiri Slaby", "jirislaby@gmail.com"));

	assert(!repo.init(gitDir));
	assert(std::filesystem::exists(gitDir / ".git/objects"));

	SlGit::Index index;
	assert(!index.repoIndex(repo));

	// ===
	SlGit::TreeBuilder aTb;
	assert(!aTb.create(repo));

	std::filesystem::path aFile{"a.txt"};
	std::string aContent{"this is " + aFile.string()};

	SlGit::Blob aBlob;
	assert(!aBlob.createFromBuffer(repo, aContent));

	assert(!aTb.insert(aFile, aBlob));
	SlGit::Tree aTree;
	assert(!aTb.write(repo, aTree));
	std::cout << "aTree=" << aTree.idStr() << '\n';

	assert(!index.readTree(aTree));

	SlGit::Commit aCommit;
	assert(!aCommit.createCheckout(repo, me, me, "commit of " + aFile.string(), aTree));
	std::cout << "aCommit=" << aCommit.idStr() << '\n';

	// ===
	SlGit::TreeBuilder bTb;
	assert(!bTb.create(repo, &aTree));

	std::filesystem::path bFile{"b.txt"};
	std::string bContent{"this is " + bFile.string()};

	SlGit::Blob bBlob;
	assert(!bBlob.createFromBuffer(repo, bContent));

	assert(!bTb.insert(bFile, bBlob));
	SlGit::Tree bTree;
	assert(!bTb.write(repo, bTree));
	std::cout << "bTree=" << bTree.idStr() << '\n';

	assert(!index.readTree(bTree));

	SlGit::Commit bCommit;
	assert(!bCommit.createCheckout(repo, me, me, "commit of " + bFile.string(), bTree,
				       GIT_CHECKOUT_SAFE | GIT_CHECKOUT_RECREATE_MISSING,
				       { &aCommit }));
	std::cout << "bCommit=" << bCommit.idStr() << '\n';

	// ===
	assert(std::filesystem::exists(gitDir / aFile));
	assert(std::filesystem::exists(gitDir / bFile));

	std::ifstream ifs;
	ifs.open(gitDir / aFile);
	assert(ifs.good());
	std::string line;
	assert(std::getline(ifs, line));
	ifs.close();
	assert(line == aContent);

	std::filesystem::remove_all(gitDir);

	return 0;
}

