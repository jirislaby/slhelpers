// SPDX-License-Identifier: GPL-2.0-only

#include <filesystem>
#include <iostream>

#include <git2.h>
#include <optional>

#include "helpers/Ratelimit.h"
#include "helpers/SSH.h"
#include "git/Git.h"

using namespace SlGit;

Repo::Repo() : m_repo(nullptr)
{
	git_libgit2_init();
}

Repo::~Repo()
{
	git_repository_free(m_repo);
	git_libgit2_shutdown();
}

int Repo::init(const std::filesystem::path &path, bool bare, const std::string &origin)
{
	git_repository_init_options opts GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
	if (bare)
		opts.flags |= GIT_REPOSITORY_INIT_BARE;
	if (!origin.empty())
		opts.origin_url = origin.c_str();
	return git_repository_init_ext(&m_repo, path.c_str(), &opts);
}

class MyFetchCallbacks : public FetchCallbacks {
public:
	MyFetchCallbacks() : ratelimit(std::chrono::seconds(2)), keys(SlSSH::Keys::get("")),
		tried(0), triedKey(0) { }

	virtual int credentials(git_credential **out, const std::string &url,
				const std::optional<std::string> &usernameFromUrl,
				unsigned int allowedTypes) override {
		auto user = getUserName(usernameFromUrl);
		std::cerr << __func__ << ": url=" << url << " user=" << user <<
			     " types=" << std::bitset<8>{allowedTypes} <<
			     " tried=" << std::bitset<8>{tried} <<
			     " keys=" << keys.size() <<
			     " triedKey=" << triedKey << '\n';

		if (allowedTypes & GIT_CREDENTIAL_USERNAME)
			return git_credential_username_new(out, user.c_str());

		if (allowedTypes & GIT_CREDENTIAL_SSH_KEY && !(tried & GIT_CREDENTIAL_SSH_KEY)) {
			if (triedKey >= keys.size()) {
				tried |= GIT_CREDENTIAL_SSH_KEY;
				return GIT_PASSTHROUGH;
			}
			const auto &keyPair = keys[triedKey++];
			return git_credential_ssh_key_new(out, user.c_str(),
							  keyPair.first.string().c_str(),
							  keyPair.second.string().c_str(), nullptr);
		}

		std::cerr << "\tUNHANDLED!\n";

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
	std::string getUserName(const std::optional<std::string> &usernameFromUrl) {
		if (!userName.empty())
			return userName;

		if (usernameFromUrl)
			return userName = *usernameFromUrl;

		return userName = ::getpwuid(::getuid())->pw_name;
	}

	std::string userName;
	SlHelpers::Ratelimit ratelimit;
	SlSSH::Keys::KeyPairs keys;
	unsigned int tried;
	unsigned int triedKey;
};

static int fetchCredentials(git_credential **out, const char *url, const char *usernameFromUrl,
		unsigned int allowedTypes, void *payload)
{
	std::optional<std::string> username;
	if (usernameFromUrl)
		username = usernameFromUrl;
	return static_cast<FetchCallbacks *>(payload)->credentials(out, url, username,
								   allowedTypes);
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
	git_clone_options opts GIT_CLONE_OPTIONS_INIT;
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
	return git_clone(&m_repo, url.c_str(), path.c_str(), &opts);
}

int Repo::open(const std::filesystem::path &path)
{
	int ret = git_repository_open(&m_repo, path.c_str());
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

	ret = checkoutTree(tree);
	if (ret)
		return ret;

	ret = git_repository_set_head(m_repo, branch.c_str());
	if (ret)
		return ret;

	return 0;
}

int Repo::checkoutTree(const Tree &tree, unsigned int strategy)
{
	git_checkout_options opts GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = strategy;
	return git_checkout_tree(m_repo, reinterpret_cast<const git_object *>(tree.tree()), &opts);
}

std::optional<std::string> Repo::catFile(const std::string &branch, const std::string &file) const
{
	Commit commit;

	if (commit.revparseSingle(*this, branch))
		return {};

	return commit.catFile(*this, file);
}

int Remote::fetchRefspecs(const std::vector<std::string> &refspecs, int depth, bool tags)
{
	if (refspecs.empty())
		return git_remote_fetch(m_remote, nullptr, nullptr, nullptr);
	git_fetch_options opts GIT_FETCH_OPTIONS_INIT;
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
	return git_remote_fetch(m_remote, StrArray(refspecs), &opts, nullptr);
}

int Remote::fetchBranches(const std::vector<std::string> &branches, int depth, bool tags)
{
	std::string remote { git_remote_name(*this) };
	std::vector<std::string> refspecs;

	for (const auto &b : branches)
		refspecs.push_back("refs/heads/" + b + ":refs/remotes/" + remote + "/" + b);

	return fetchRefspecs(refspecs, depth, tags);
}

int Tree::walkCB(const char *root, const git_tree_entry *entry, void *payload)
{
	const auto CB = *static_cast<Tree::WalkCallback *>(payload);

	TreeEntry treeEntry(entry);
	return CB(root, treeEntry);
}

int Tree::walk(const WalkCallback &CB, const git_treewalk_mode &mode) {
	return git_tree_walk(m_tree, mode, walkCB,
			     const_cast<void *>(static_cast<const void *>(&CB)));
}

std::optional<std::string> Tree::catFile(const Repo &repo, const std::string &file) const
{
	TreeEntry treeEntry;
	if (treeEntry.byPath(*this, file))
		return {};

	return treeEntry.catFile(repo);
}

std::optional<std::string> Commit::catFile(const Repo &repo, const std::string &file) const
{
	Tree tree;
	if (tree.ofCommit(*this))
		return {};

	return tree.catFile(repo, file);
}

std::optional<std::string> TreeEntry::catFile(const Repo &repo) const
{
	if (type() != GIT_OBJECT_BLOB)
		return {};

	Blob blob;
	if (blob.lookup(repo, *this))
		return {};

	return blob.content();
}

int TreeBuilder::insert(const std::filesystem::path &file, const Blob &blob)
{
	const git_tree_entry *te;
	return git_treebuilder_insert(&te, m_treeBuilder, file.c_str(), blob.id(),
				      GIT_FILEMODE_BLOB);
}

int Index::addAll(const std::vector<std::string> &paths, unsigned int flags, const MatchCB &cb)
{
	return git_index_add_all(m_index, StrArray(paths), flags, matchCB,
				 const_cast<void *>(static_cast<const void *>(&cb)));
}

int Index::removeAll(const std::vector<std::string> &paths, const MatchCB &cb)
{
	return git_index_remove_all(m_index, StrArray(paths), matchCB,
				 const_cast<void *>(static_cast<const void *>(&cb)));
}

int Index::updateAll(const std::vector<std::string> &paths, const MatchCB &cb)
{
	return git_index_update_all(m_index, StrArray(paths), matchCB,
				 const_cast<void *>(static_cast<const void *>(&cb)));
}

int Index::matchCB(const char *path, const char *matched_pathspec, void *payload)
{
	const auto &cb = *static_cast<const MatchCB *>(payload);

	return cb(path, matched_pathspec);
}
