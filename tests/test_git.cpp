#include <cassert>
#include <iostream>
#include <fstream>

#include "git/Git.h"

#include "helpers.h"

static SlGit::Repo testRepoInit()
{
	auto gitDir = THelpers::getTmpDir("testgitdir");

	auto repoOpt = SlGit::Repo::init(gitDir);
	assert(repoOpt);
	auto repo = std::move(*repoOpt);
	assert(std::filesystem::exists(gitDir / ".git/objects"));
	std::cout << __func__ << ": gitDir=" << gitDir << " workDir=" << repo.workDir() << '\n';
	assert(std::filesystem::equivalent(gitDir, repo.workDir()));

	assert(repo.index());

	return repo;
}

static SlGit::Repo testRepoClone(const SlGit::Repo &repo)
{
	auto gitDir2 = THelpers::getTmpDir("testgitdir2");
	std::cout << __func__ << ": gitDir2=" << gitDir2 << '\n';
	std::cout << "vvv clone output vvv\n";
	auto repo2 = SlGit::Repo::clone(gitDir2, repo.path());
	std::cout << "^^^ clone output ^^^\n";
	assert(repo2);

	return std::move(*repo2);
}

static SlGit::Signature testSignature()
{
	static const std::string name("Jiri Slaby");
	static const std::string email("jirislaby@gmail.com");
	auto me = SlGit::Signature::now(name, email);
	assert(me);
	assert(me->name() == name);
	assert(me->email() == email);
	return std::move(*me);
}

static std::tuple<SlGit::Commit, std::filesystem::path, std::string>
createACommit(const SlGit::Repo &repo, const SlGit::Signature &me)
{
	auto aTb = repo.treeBuilderCreate();
	assert(aTb);

	std::filesystem::path aFile{"a.txt"};
	std::string aContent{"this is " + aFile.string()};

	auto aBlob = repo.blobCreateFromBuffer(aContent);
	assert(aBlob);
	assert(aBlob->type() == GIT_OBJECT_BLOB);
	assert(aBlob->typeStr() == "blob");

	assert(!aTb->insert(aFile, *aBlob));
	auto aTreeOpt = aTb->write(repo);
	assert(aTreeOpt);
	auto aTree = std::move(*aTreeOpt);
	assert(aTree.type() == GIT_OBJECT_TREE);
	assert(aTree.typeStr() == "tree");
	std::cout << __func__ << ": aTree=" << aTree.idStr() << '\n';

	assert(!repo.index()->readTree(aTree));

	auto aCommitOpt = repo.commitCreateCheckout(me, me, "commit of " + aFile.string(), aTree);
	assert(aCommitOpt);
	auto aCommit = std::move(*aCommitOpt);
	assert(aCommit.type() == GIT_OBJECT_COMMIT);
	assert(aCommit.typeStr() == "commit");
	std::cout << __func__ << ": aCommit=" << aCommit.idStr() << '\n';

	assert(repo.refCreateDirect("refs/heads/aRef", *aCommit.id()));
	assert(repo.refCreateDirect("refs/heads/aRef2", *aCommit.id()));

	return { std::move(aCommit), aFile, aContent };
}

static std::tuple<SlGit::Commit, std::filesystem::path, std::string>
createBCommit(const SlGit::Repo &repo, const SlGit::Commit &aCommit, const SlGit::Signature &me)
{
	auto aTree = *aCommit.tree();
	auto bTb = repo.treeBuilderCreate(&aTree);
	assert(bTb);

	std::filesystem::path bFile{"b.txt"};
	std::string bContent{"this is " + bFile.string()};

	auto bBlob = repo.blobCreateFromBuffer(bContent);
	assert(bBlob);

	assert(!bTb->insert(bFile, *bBlob));
	auto bTreeOpt = bTb->write(repo);
	assert(bTreeOpt);
	auto bTree = std::move(*bTreeOpt);
	std::cout << __func__ << ": bTree=" << bTree.idStr() << '\n';

	assert(!repo.index()->readTree(bTree));

	auto bCommitOpt = repo.commitCreateCheckout(me, me, "commit of " + bFile.string(), bTree,
						    GIT_CHECKOUT_SAFE | GIT_CHECKOUT_RECREATE_MISSING,
						    { &aCommit });
	assert(bCommitOpt);
	auto bCommit = std::move(*bCommitOpt);
	assert(bCommit.type() == GIT_OBJECT_COMMIT);
	std::cout << __func__ << ": bCommit=" << bCommit.idStr() << '\n';

	assert(repo.refCreateDirect("refs/heads/bRef", *bCommit.id()));

	return { std::move(bCommit), bFile, bContent };
}

static void testRefs(const SlGit::Repo &repo, const SlGit::Commit &aCommit)
{
	auto ref = repo.refDWIM("aRef2");
	assert(ref);
	auto idStr = SlGit::Helpers::oidToStr(*ref->target());
	assert(idStr == aCommit.idStr());
	std::cout << __func__ << ": aRef2 points to " << idStr << '\n';
	ref = repo.refDWIM("aRef3");
	assert(!ref);
}

static void testOperator(const SlGit::Commit &aCommit, const SlGit::Commit &bCommit)
{
	assert(aCommit == aCommit);
	assert(bCommit == bCommit);
	assert(!(aCommit == bCommit));
	assert(aCommit != bCommit);
}

static void testDiff(const SlGit::Repo &repo, const SlGit::Commit &aCommit,
		     const SlGit::Commit &bCommit)
{
	auto diff = repo.diff(aCommit, bCommit);
	assert(diff);
	assert(!diff->print(GIT_DIFF_FORMAT_PATCH,
			    [](const git_diff_delta &delta,
			       const git_diff_hunk *hunk,
			       const git_diff_line &line) -> int {
		std::cout << delta.old_file.path << "->" << delta.new_file.path << '\n';
		if (hunk)
			std::cout << "hunk=" << hunk->header << '\n';
		std::string_view s(line.content, line.content_len);
		std::cout << "origin=" << line.origin << " content=" << s << "=========\n";
		return 0;
	}));
}

static void testTags(const SlGit::Repo &repo, const SlGit::Commit &aCommit,
		     const SlGit::Commit &bCommit, const SlGit::Signature &me)
{
	auto tag = repo.tagCreate("aTag", aCommit, me, "a tag for aCommit");
	assert(tag);
	assert(tag->type() == GIT_OBJECT_TAG);
	assert(tag->typeStr() == "tag");
	tag = repo.tagCreate("bTag", bCommit, me, "a tag for bCommit");
	assert(tag);
	auto aTag = repo.tagRevparseSingle("aTag");
	assert(aTag);
	assert(aTag->targetIdStr() == aCommit.idStr());

	tag = repo.tagCreate("bTreeTag", *bCommit.tree(), me, "a tag for bTree");
	assert(tag);
	assert(tag->targetIdStr() == bCommit.tree()->idStr());
}

static void testRevparse(const SlGit::Repo &repo, const SlGit::Commit &aCommit,
			 const SlGit::Commit &bCommit, const std::filesystem::path &bFile)
{
	auto commit = repo.commitRevparseSingle("HEAD");
	assert(commit);
	assert(commit == repo.commitHead());
	assert(commit == bCommit);
	assert(commit->parent() == aCommit);

	commit = repo.commitRevparseSingle("HEAD^1");
	assert(commit);
	assert(commit == aCommit);

	auto failTree = repo.commitRevparseSingle("HEAD^{tree}");
	assert(!failTree);
	auto failBlob = repo.commitRevparseSingle("HEAD:" + bFile.string());
	assert(!failBlob);

	auto tree = repo.treeRevparseSingle("HEAD^{tree}");
	assert(tree);
	assert(tree == bCommit.tree());

	auto blob = repo.blobRevparseSingle("HEAD:" + bFile.string());
	assert(blob);
	assert(blob->idStr() == bCommit.tree()->treeEntryByPath(bFile)->idStr());
}

static void testRemote(const SlGit::Repo &repo)
{
	const std::string url = "https://localhost";
	auto remote1 = repo.remoteCreate("origin", url);
	assert(remote1);
	auto remote2 = repo.remoteLookup("origin");
	assert(remote2);
	assert(remote1->url() == url);
	assert(remote1->url() == remote2->url());
	std::cout << __func__ << ": remote1=" << remote1->url() <<
		     " remote2=" << remote2->url() << '\n';
	remote2 = repo.remoteLookup("origin2");
	assert(!remote2);
}

static void testRevWalk(const SlGit::Repo &repo, const SlGit::Commit &aCommit,
			const SlGit::Commit &bCommit)
{
	auto revWalk = repo.revWalkCreate();
	assert(revWalk);
	revWalk->pushHead();
	auto nextCommit = revWalk->next(repo);
	assert(nextCommit);
	assert(nextCommit == bCommit);
	std::cout << __func__ << ": top-0=" << nextCommit->idStr() << '\n';
	nextCommit = revWalk->next(repo);
	assert(nextCommit);
	assert(nextCommit == aCommit);
	std::cout << __func__ << ": top-1=" << nextCommit->idStr() << '\n';
	nextCommit = revWalk->next(repo);
	assert(!nextCommit);
}

static void testCatFile(const SlGit::Repo &repo, const SlGit::Commit &aCommit,
			const std::filesystem::path &aFile, const std::string &aContent,
			const std::filesystem::path &bFile, const std::string &bContent)
{
	auto aContentRead = aCommit.catFile(repo, aFile);
	assert(aContentRead);
	assert(aContent == *aContentRead);

	auto noBFile = aCommit.catFile(repo, bFile);
	assert(!noBFile);

	auto bContentRead = repo.catFile("HEAD", bFile);
	assert(bContentRead);
	assert(bContent == *bContentRead);
}

static void testFilesOnFS(const SlGit::Repo &repo, const std::filesystem::path &aFile,
			  const std::string &aContent,
			  const std::filesystem::path &bFile)
{
	auto gitDir = repo.workDir();

	assert(std::filesystem::exists(gitDir / aFile));
	assert(std::filesystem::exists(gitDir / bFile));

	std::ifstream ifs;
	ifs.open(gitDir / aFile);
	assert(ifs.good());
	std::string line;
	assert(std::getline(ifs, line));
	ifs.close();
	assert(line == aContent);
}
static void testCheckout(const SlGit::Repo &repo2, const SlGit::Commit &aCommit)
{
	auto r = repo2.refDWIM("origin/aRef2");
	assert(r);
	assert(r->name() == "refs/remotes/origin/aRef2");
	assert(!repo2.checkout(*r));
	auto head = repo2.commitRevparseSingle("HEAD");
	assert(head);
	std::cout << __func__ << ": cloned head=" << head->idStr() << '\n';
	assert(head == aCommit);
}

static void testFetch(const SlGit::Repo &repo2, const SlGit::Commit &bCommit)
{
	auto remote = repo2.remoteLookup("origin");
	assert(remote);
	std::cout << "vvv fetch output vvv\n";
	assert(!remote->fetch("master"));
	std::cout << "^^^ fetch output ^^^\n";
	auto originMaster = repo2.commitRevparseSingle("origin/master");
	std::cout << __func__ << ": origin/master=" << originMaster->idStr() << '\n';
	assert(originMaster == bCommit);
}

int main()
{
	auto repo = testRepoInit();
	auto me = testSignature();
	auto [ aCommit, aFile, aContent ] = createACommit(repo, me);
	testRefs(repo, aCommit);
	auto repo2 = testRepoClone(repo);
	auto [ bCommit, bFile, bContent ] = createBCommit(repo, aCommit, me);
	testOperator(aCommit, bCommit);
	testDiff(repo, aCommit, bCommit);
	testTags(repo, aCommit, bCommit, me);
	testRevparse(repo, aCommit, bCommit, bFile);
	testRemote(repo);
	testRevWalk(repo, aCommit, bCommit);
	testCatFile(repo, aCommit, aFile, aContent, bFile, bContent);
	testFilesOnFS(repo, aFile, aContent, bFile);
	testCheckout(repo2, aCommit);
	testFetch(repo2, bCommit);

	std::filesystem::remove_all(repo.workDir());
	std::filesystem::remove_all(repo2.workDir());

	return 0;
}

