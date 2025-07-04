#include <filesystem>
#include <iostream>

#include <git2.h>
#include <optional>

#include "helpers/Ratelimit.h"
#include "helpers/SSH.h"
#include "git/Git.h"

using namespace SlGit;

Repo::Repo() : repo(nullptr)
{
	git_libgit2_init();
}

Repo::~Repo()
{
	git_repository_free(repo);
	git_libgit2_shutdown();
}

int Repo::init(const std::filesystem::path &path, bool bare, const std::string &origin)
{
	git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
	if (bare)
		opts.flags |= GIT_REPOSITORY_INIT_BARE;
	if (!origin.empty())
		opts.origin_url = origin.c_str();
	return git_repository_init_ext(&repo, path.c_str(), &opts);
}

class MyFetchCallbacks : public FetchCallbacks {
public:
	MyFetchCallbacks() : ratelimit(std::chrono::seconds(2)), tried(0) { }

	virtual int credentials(git_credential **out, const std::string &url,
				const std::optional<std::string> &username_from_url,
				unsigned int allowed_types) override {
		std::cerr << __func__ << ": url=" << url << " user=" <<
			     (username_from_url ? *username_from_url : "NULL") <<
			     " types=" << std::bitset<8>{allowed_types} <<
			     " tried=" << std::bitset<8>{tried} << '\n';
		if (allowed_types & GIT_CREDENTIAL_SSH_KEY && !(tried & GIT_CREDENTIAL_SSH_KEY)) {
			tried |= GIT_CREDENTIAL_SSH_KEY;
			SlSSH::Keys::get("");
			return git_credential_ssh_key_new(out,
							  username_from_url ?
								  username_from_url->c_str() :
								  nullptr,
							  "/home/xslaby/.ssh/id_rsa.pub",
							  "/home/xslaby/.ssh/id_rsa", nullptr);
		}

		return GIT_PASSTHROUGH;
	}
	virtual int packProgress(int stage, uint32_t current, uint32_t total) override {
		if (!ratelimit.limit() && current != 0 && current != total)
			return 0;
		std::cerr << __func__ << ": stage=" << stage << " " << current << "/" << total << '\n';
		return 0;
	}
	virtual int sidebandProgress(const std::string_view &str) override {
		if (!ratelimit.limit())
			return 0;
		std::cerr << __func__ << ": " << str;
		if (str.back() != '\n')
			std::cerr << '\n';
		return 0;
	}
	virtual int transferProgress(const git_indexer_progress &stats) override {
		if (!ratelimit.limit() && stats.indexed_objects != 0 &&
				stats.indexed_objects != stats.total_objects)
			return 0;
		std::cerr << std::fixed << std::setprecision(2) << __func__ <<
			     ": deltas=" << stats.indexed_deltas << '/' << stats.total_deltas <<
			     " objs=" << stats.indexed_objects << '/' << stats.total_objects <<
			     " local=" << stats.local_objects <<
			     " recv=" << stats.received_objects <<
			     " (" << stats.received_bytes / 1024. << " kB)\n";
		return 0;
	}
	virtual int updateRefs(const std::string &refname, const git_oid &a, const git_oid &b,
			       git_refspec &) override {
		char sha1[12] = {0}, sha2[12] = {0};
		git_oid_tostr(sha1, sizeof(sha1) - 1, &a);
		git_oid_tostr(sha2, sizeof(sha2) - 1, &b);
		std::cerr << __func__ << ": ref=" << refname << " " << sha1 << ".." << sha2 << '\n';
		return 0;
	}

private:
	SlHelpers::Ratelimit ratelimit;
	unsigned int tried;
};

static int fetchCredentials(git_credential **out, const char *url, const char *username_from_url,
		unsigned int allowed_types, void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->credentials(out, url, username_from_url ? : "",
							    allowed_types);
}

static int fetchPackProgress(int stage, uint32_t current, uint32_t total, void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->packProgress(stage, current, total);
}

static int fetchSidebandProgress(const char *str, int len, void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->sidebandProgress({ str, static_cast<size_t>(len) });
}

static int fetchTransferProgress(const git_indexer_progress *stats, void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->transferProgress(*stats);
}

#ifdef LIBGIT_HAS_UPDATE_REFS
static int fetchUpdateRefs(const char *refname, const git_oid *a, const git_oid *b, git_refspec *refspec,
		      void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->updateRefs(refname, *a, *b, *refspec);
}
#endif

int Repo::clone(const std::filesystem::path &path, const std::string &url,
		const std::string &branch, const unsigned int &depth, bool tags)
{
	MyFetchCallbacks fc;
	git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
	opts.checkout_branch = branch.empty() ? nullptr : branch.c_str();
	opts.fetch_opts.depth = depth;
	if (!tags)
		opts.fetch_opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_NONE;
	opts.fetch_opts.callbacks.payload = &fc;
	opts.fetch_opts.callbacks.credentials = fetchCredentials;
	opts.fetch_opts.callbacks.pack_progress = fetchPackProgress;
	opts.fetch_opts.callbacks.sideband_progress = fetchSidebandProgress;
	opts.fetch_opts.callbacks.transfer_progress = fetchTransferProgress;
#ifdef LIBGIT_HAS_UPDATE_REFS
	opts.fetch_opts.callbacks.update_refs = fetchUpdateRefs;
#endif
	return git_clone(&repo, url.c_str(), path.c_str(), &opts);
}

int Repo::open(const std::filesystem::path &path)
{
	int ret = git_repository_open(&repo, path.c_str());
	if (ret < 0) {
		auto e = git_error_last();
		std::cerr << "Git: error " << ret << "/" << e->klass << ": " << e->message << '\n';
		return -1;
	}

	return 0;
}

int Repo::checkout(const std::string &branch)
{
	Reference ref;
	int ret = ref.lookup(*this, branch);
	if (ret)
		return ret;

	Commit c;
	ret = c.lookup(*this, *ref.target());
	if (ret)
		return ret;

	Tree tree;
	ret = tree.ofCommit(c);
	if (ret)
		return ret;

	auto obj = reinterpret_cast<git_object *>(static_cast<git_tree *>(tree));
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	ret = git_checkout_tree(*this, obj, &opts);
	if (ret)
		return ret;

	ret = git_repository_set_head(repo, branch.c_str());
	if (ret)
		return ret;

	return 0;
}

std::optional<std::string> Repo::catFile(const std::string &branch, const std::string &file)
{
	Commit commit;

	if (commit.revparseSingle(*this, branch))
		return {};

	Tree tree;
	if (tree.ofCommit(commit))
		return {};

	unsigned int cnt = 0;
	auto ret = tree.walk([&cnt](const std::string &root, const TreeEntry &entry) -> int {
		      if (++cnt == 10)
			      return -1000;
		      std::cout << "walk: root=" << root << " E=" << entry.name() << '\n';
		      return 0;
	});
	if (ret && ret != -1000)
		return {};

	TreeEntry treeEntry;
	if (treeEntry.byPath(tree, file))
		return {};

	Blob blob;
	if (blob.lookup(*this, treeEntry))
		return {};

	return blob.content();
}

int Remote::fetchRefspecs(const std::vector<std::string> &refspecs, int depth, bool tags)
{
	if (refspecs.empty())
		return git_remote_fetch(remote, nullptr, nullptr, nullptr);
	std::vector<char *> strings;
	for (const auto &r : refspecs)
		strings.push_back(const_cast<char *>(r.c_str()));
	git_strarray refs = {
		.strings = strings.data(),
		.count = strings.size(),
	};
	git_fetch_options opts = GIT_FETCH_OPTIONS_INIT;
	MyFetchCallbacks fc;
	opts.callbacks.payload = &fc;
	opts.callbacks.credentials = fetchCredentials;
	opts.callbacks.pack_progress = fetchPackProgress;
	opts.callbacks.sideband_progress = fetchSidebandProgress;
	opts.callbacks.transfer_progress = fetchTransferProgress;
#ifdef LIBGIT_HAS_UPDATE_REFS
	opts.callbacks.update_refs = fetchUpdateRefs;
#endif
	if (!tags)
		opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_NONE;
	opts.depth = depth;
	return git_remote_fetch(remote, &refs, &opts, nullptr);
}

int Remote::fetchBranches(const std::vector<std::string> &branches, int depth, bool tags)
{
	std::string remote { git_remote_name(*this) };
	std::vector<std::string> refspecs;

	for (const auto &b : branches)
		refspecs.push_back("refs/heads/" + b + ":refs/remotes/" + remote + "/" + b);

	return fetchRefspecs(refspecs, depth, tags);
}

static int treeWalkCB(const char *root, const git_tree_entry *entry, void *payload)
{
	const auto CB = *static_cast<Tree::WalkCallback *>(payload);

	TreeEntry treeEntry(entry);
	return CB(root, treeEntry);
}

int Tree::walk(const WalkCallback &CB, const git_treewalk_mode &mode) {
	return git_tree_walk(tree, mode, treeWalkCB,
			     const_cast<void *>(static_cast<const void *>(&CB)));
}
