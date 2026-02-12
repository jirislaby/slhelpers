// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>
#include <iostream>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Diff.h"
#include "git/Index.h"
#include "git/Repo.h"
#include "git/Remote.h"
#include "git/Misc.h"
#include "git/Tag.h"
#include "git/Tree.h"

#include "helpers/Misc.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_repository>::operator()(git_repository *repo) const
{
	git_repository_free(repo);
}

thread_local SlHelpers::LastErrorStr<int, int> Repo::m_lastError;

std::optional<Repo> Repo::init(const std::filesystem::path &path, bool bare,
				 const std::string &originUrl) noexcept
{
	git_repository_init_options opts GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
	if (bare)
		opts.flags |= GIT_REPOSITORY_INIT_BARE;
	if (!originUrl.empty())
		opts.origin_url = originUrl.c_str();

	return MakeGit<Repo>(git_repository_init_ext, path.c_str(), &opts);
}

std::optional<Repo> Repo::open(const std::filesystem::__cxx11::path &path) noexcept
{
	return MakeGit<Repo>(git_repository_open, path.c_str());
}

bool Repo::update(const std::filesystem::path &path, const std::string &remote)
{
	std::cerr << "Trying to fetch... " << remote << " in " << path << '\n';
	auto repoOpt = SlGit::Repo::open(path);
	if (!repoOpt)
		return false;

	auto remoteOpt = repoOpt->remoteLookup(remote);
	if (!remoteOpt)
		return false;

	if (!remoteOpt->fetchRefspecs())
		return false;

	const auto stats = remoteOpt->stats();
	if (stats->local_objects > 0)
		std::cerr << "Received " << stats->indexed_objects << '/' <<
			     stats->total_objects << " objects in " <<
			     SlHelpers::Unit::human(stats->received_bytes) <<
			     " (used " << stats->local_objects << " local objects)\n";
	else
		std::cerr << "Received " << stats->indexed_objects << '/' <<
			     stats->total_objects << " objects in " <<
			     SlHelpers::Unit::human(stats->received_bytes) << '\n';

	return true;
}

bool Repo::checkoutTree(const Tree &tree, unsigned int strategy) const noexcept
{
	git_checkout_options opts GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = strategy;
	return !setLastError(git_checkout_tree(repo(),
					       reinterpret_cast<const git_object *>(tree.tree()),
					       &opts));
}

std::optional<std::string> Repo::catFile(const std::string &branch,
					 const std::string &file) const noexcept
{
	if (auto commit = commitRevparseSingle(branch))
		return commit->catFile(file);

	return std::nullopt;
}

std::optional<Blob> Repo::blobCreateFromWorkDir(const std::filesystem::path &file) const noexcept
{
	git_oid oid;
	if (setLastError(git_blob_create_from_workdir(&oid, repo(), file.c_str())))
		return std::nullopt;
	return blobLookup(oid);
}

std::optional<Blob> Repo::blobCreateFromDisk(const std::filesystem::path &file) const noexcept
{
	git_oid oid;
	if (setLastError(git_blob_create_from_disk(&oid, repo(), file.c_str())))
		return std::nullopt;
	return blobLookup(oid);
}

std::optional<Blob> Repo::blobCreateFromBuffer(const std::string &buf) const noexcept
{
	git_oid oid;
	if (setLastError(git_blob_create_from_buffer(&oid, repo(), buf.c_str(), buf.length())))
		return std::nullopt;
	return blobLookup(oid);
}

std::optional<Blob> Repo::blobLookup(const git_oid &oid) const noexcept
{
	return MakeGitRepo<Blob>(*this, git_blob_lookup, repo(), &oid);
}

std::optional<Blob> Repo::blobLookup(const TreeEntry &tentry) const noexcept
{
	return blobLookup(*tentry.id());
}

std::optional<Blob> Repo::blobRevparseSingle(const std::string &rev) const noexcept
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Blob>(res))
		return std::move(std::get<Blob>(res));
	return std::nullopt;
}

std::optional<Commit> Repo::commitLookup(const git_oid &oid) const noexcept
{
	return MakeGitRepo<Commit>(*this, git_commit_lookup, repo(), &oid);
}

std::optional<Commit> Repo::commitCreate(const Signature &author, const Signature &committer,
					 const std::string &msg, const Tree &tree,
					 const std::vector<const Commit *> &parents) const noexcept
{
	git_oid oid;
	std::vector<const git_commit *> parentPtrs;

	for (const auto &p : parents)
		parentPtrs.push_back(p->commit());

	if (setLastError(git_commit_create(&oid, repo(), "HEAD", author, committer, "UTF-8",
					   msg.c_str(), tree, parentPtrs.size(),
					   parentPtrs.data())))
		return std::nullopt;
	return commitLookup(oid);
}

std::optional<Commit> Repo::commitCreateCheckout(const Signature &author,
						 const Signature &committer,
						 const std::string &msg, const Tree &tree,
						 unsigned int strategy,
						 const std::vector<const Commit *> &parents) const noexcept
{
	auto commit = commitCreate(author, committer, msg, tree, parents);
	if (!commit)
		return std::nullopt;
	if (!checkoutTree(tree, strategy))
		return std::nullopt;
	return commit;
}

std::optional<Commit> Repo::commitHead() const noexcept
{
	return commitRevparseSingle("HEAD");
}

std::optional<Commit> Repo::commitRevparseSingle(const std::string &rev) const noexcept
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Commit>(res))
		return std::move(std::get<Commit>(res));
	return std::nullopt;
}

std::optional<Diff> Repo::diff(const Commit &commit1, const Commit &commit2,
			       const git_diff_options *opts) const noexcept
{
	return diff(*commit1.tree(), *commit2.tree(), opts);
}

std::optional<Diff> Repo::diff(const Tree &tree1, const Tree &tree2,
			       const git_diff_options *opts) const noexcept
{
	return MakeGit<Diff>(git_diff_tree_to_tree, repo(), tree1, tree2, opts);
}

std::optional<Diff> Repo::diffCached(const Commit &commit, const Index &index,
				     const git_diff_options *opts) const noexcept
{
	return diffCached(*commit.tree(), index, opts);
}

std::optional<Diff> Repo::diffCached(const Tree &tree, const Index &index,
				     const git_diff_options *opts) const noexcept
{
	return MakeGit<Diff>(git_diff_tree_to_index, repo(), tree, index, opts);
}

std::optional<Diff> Repo::diffCached(const Commit &commit,
				     const git_diff_options *opts) const noexcept
{
	return diffCached(commit, *index(), opts);
}

std::optional<Diff> Repo::diffCached(const Tree &tree, const git_diff_options *opts) const noexcept
{
	return diffCached(tree, *index(), opts);
}

std::optional<Diff> Repo::diffWorkdir(const Index &index,
				      const git_diff_options *opts) const noexcept
{
	return MakeGit<Diff>(git_diff_index_to_workdir, repo(), index, opts);
}

std::optional<Diff> Repo::diffWorkdir(const Commit &commit,
				      const git_diff_options *opts) const noexcept
{
	return diffWorkdir(*commit.tree(), opts);
}

std::optional<Diff> Repo::diffWorkdir(const Tree &tree, const git_diff_options *opts) const noexcept
{
	return MakeGit<Diff>(git_diff_tree_to_workdir, repo(), tree, opts);
}

std::variant<Blob, Commit, Tag, Tree, std::monostate>
Repo::revparseSingle(const std::string &rev) const noexcept
{
	git_object *obj;
	if (setLastError(git_revparse_single(&obj, repo(), rev.c_str())))
		return std::monostate{};

	switch (git_object_type(obj)) {
	case GIT_OBJECT_BLOB:
		return Blob(*this, reinterpret_cast<git_blob *>(obj));
	case GIT_OBJECT_COMMIT:
		return Commit(*this, reinterpret_cast<git_commit *>(obj));
	case GIT_OBJECT_TAG:
		return Tag(*this, reinterpret_cast<git_tag *>(obj));
	case GIT_OBJECT_TREE:
		return Tree(*this, reinterpret_cast<git_tree *>(obj));
	default:
		git_object_free(obj);
		return std::monostate{};
	}
}

std::optional<Tree> Repo::treeLookup(const git_oid &oid) const noexcept
{
	return MakeGitRepo<Tree>(*this, git_tree_lookup, repo(), &oid);
}

std::optional<Tree> Repo::treeLookup(const TreeEntry &tentry) const noexcept
{
	return treeLookup(*tentry.id());
}

std::optional<Tree> Repo::treeRevparseSingle(const std::string &rev) const noexcept
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Tree>(res))
		return std::move(std::get<Tree>(res));
	return std::nullopt;
}

std::optional<Index> Repo::index() const noexcept
{
	return MakeGit<Index>(git_repository_index, repo());
}

std::optional<Remote> Repo::remoteCreate(const std::string &name,
					 const std::string &url) const noexcept
{
	return MakeGit<Remote>(git_remote_create, repo(), name.c_str(), url.c_str());
}

std::optional<Remote> Repo::remoteLookup(const std::string &name) const noexcept
{
	return MakeGit<Remote>(git_remote_lookup, repo(), name.c_str());
}

std::optional<RevWalk> Repo::revWalkCreate() const noexcept
{
	return MakeGitRepo<RevWalk>(*this, git_revwalk_new, repo());
}

std::optional<Tag> Repo::tagCreate(const std::string &tagName, const Object &target,
				   const Signature &tagger, const std::string &message,
				   bool force) const noexcept
{
	git_oid oid;
	if (setLastError(git_tag_create(&oid, repo(), tagName.c_str(), target.object(), tagger,
					message.c_str(), force)))
		return std::nullopt;
	return tagLookup(oid);
}

std::optional<Tag> Repo::tagLookup(const git_oid &oid) const noexcept
{
	return MakeGitRepo<Tag>(*this, git_tag_lookup, repo(), &oid);
}

std::optional<Tag> Repo::tagLookup(const TreeEntry &tentry) const noexcept
{
	return tagLookup(*tentry.id());
}

std::optional<Tag> Repo::tagRevparseSingle(const std::string &rev) const noexcept
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Tag>(res))
		return std::move(std::get<Tag>(res));
	return std::nullopt;
}

std::optional<Reference> Repo::refLookup(const std::string &name) const noexcept
{
	return MakeGit<Reference>(git_reference_lookup, repo(), name.c_str());
}

std::optional<Reference> Repo::refDWIM(const std::string &name) const noexcept
{
	return MakeGit<Reference>(git_reference_dwim, repo(), name.c_str());
}

std::optional<Reference> Repo::refCreateDirect(const std::string &name, const git_oid &oid,
					       bool force) const noexcept {
	return MakeGit<Reference>(git_reference_create, repo(), name.c_str(), &oid, force, nullptr);
}

std::optional<Reference> Repo::refCreateSymbolic(const std::string &name, const std::string &target,
						 bool force) const noexcept
{
	return MakeGit<Reference>(git_reference_symbolic_create, repo(), name.c_str(),
				  target.c_str(), force, nullptr);
}

std::optional<TreeBuilder> Repo::treeBuilderCreate(const Tree *source) const noexcept
{
	return MakeGit<TreeBuilder>(git_treebuilder_new, repo(), source ? source->tree() : nullptr);
}

std::pair<std::string, int> Repo::lastGitError() noexcept
{
	const auto err = git_error_last();
	/* libgit2 before 1.8 can return nullptr */
	if (!err)
		return {};
	return { err->message, err->klass };
}

void Repo::checkoutProgress(const char *path, size_t completed_steps, size_t total_steps,
			    void *payload)
{
	return static_cast<FetchCallbacks *>(payload)->checkoutProgress(path ? : "",
									completed_steps,
									total_steps);
}

std::optional<Repo> Repo::clone(const std::filesystem::path &path, const std::string &url,
				FetchCallbacks &fc, const std::string &branch,
				const unsigned int &depth, bool tags) noexcept
{
	git_clone_options opts GIT_CLONE_OPTIONS_INIT;
	opts.checkout_branch = branch.empty() ? nullptr : branch.c_str();
	opts.fetch_opts.depth = depth;
	if (!tags)
		opts.fetch_opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_NONE;
	opts.checkout_opts.progress_payload = &fc;
	opts.checkout_opts.progress_cb = checkoutProgress;
	opts.fetch_opts.callbacks.payload = &fc;
	opts.fetch_opts.callbacks.credentials = Remote::fetchCredentials;
	opts.fetch_opts.callbacks.pack_progress = Remote::fetchPackProgress;
	opts.fetch_opts.callbacks.sideband_progress = Remote::fetchSidebandProgress;
	opts.fetch_opts.callbacks.transfer_progress = Remote::fetchTransferProgress;
#ifdef LIBGIT_HAS_UPDATE_REFS
	opts.fetch_opts.callbacks.update_refs = Remote::fetchUpdateRefs;
#endif

	return MakeGit<Repo>(git_clone, url.c_str(), path.c_str(), &opts);
}

bool Repo::checkout(const std::string &branch) const noexcept
{
	auto ref = refLookup(branch);
	if (!ref)
		return false;

	return checkout(*ref);
}

bool Repo::checkout(const Reference &ref) const noexcept
{
	auto c = commitLookup(*ref.target());
	if (!c)
		return false;

	auto tree = c->tree();
	if (!tree)
		return false;

	if (!checkoutTree(*tree))
		return false;

	if (setLastError(git_repository_set_head(repo(), ref.name().c_str())))
		return false;

	return true;
}
