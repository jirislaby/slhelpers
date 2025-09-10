// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Index.h"
#include "git/Repo.h"
#include "git/Remote.h"
#include "git/Misc.h"
#include "git/Tag.h"
#include "git/Tree.h"

#include "MyFetchCallbacks.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_repository>::operator()(git_repository *repo) const
{
	git_repository_free(repo);
}

std::optional<Repo> Repo::init(const std::filesystem::path &path, bool bare,
				 const std::string &originUrl)
{
	git_repository_init_options opts GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
	if (bare)
		opts.flags |= GIT_REPOSITORY_INIT_BARE;
	if (!originUrl.empty())
		opts.origin_url = originUrl.c_str();

	return MakeGit<Repo>(git_repository_init_ext, path.c_str(), &opts);
}

std::optional<Repo> Repo::open(const std::filesystem::__cxx11::path &path)
{
	return MakeGit<Repo>(git_repository_open, path.c_str());
}

int Repo::checkoutTree(const Tree &tree, unsigned int strategy) const
{
	git_checkout_options opts GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = strategy;
	return git_checkout_tree(repo(), reinterpret_cast<const git_object *>(tree.tree()), &opts);
}

std::optional<std::string> Repo::catFile(const std::string &branch, const std::string &file) const
{
	if (auto commit = commitRevparseSingle(branch))
		return commit->catFile(*this, file);

	return std::nullopt;
}

std::optional<Blob> Repo::blobCreateFromWorkDir(const std::filesystem::path &file) const
{
	git_oid oid;
	if (git_blob_create_from_workdir(&oid, repo(), file.c_str()))
		return std::nullopt;
	return blobLookup(oid);
}

std::optional<Blob> Repo::blobCreateFromDisk(const std::filesystem::path &file) const
{
	git_oid oid;
	if (git_blob_create_from_disk(&oid, repo(), file.c_str()))
		return std::nullopt;
	return blobLookup(oid);
}

std::optional<Blob> Repo::blobCreateFromBuffer(const std::string &buf) const
{
	git_oid oid;
	if (git_blob_create_from_buffer(&oid, repo(), buf.c_str(), buf.length()))
		return std::nullopt;
	return blobLookup(oid);
}

std::optional<Blob> Repo::blobLookup(const git_oid &oid) const
{
	return MakeGit<Blob>(git_blob_lookup, repo(), &oid);
}

std::optional<Blob> Repo::blobLookup(const TreeEntry &tentry) const
{
	return blobLookup(*tentry.id());
}

std::optional<Blob> Repo::blobRevparseSingle(const std::string &rev) const
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Blob>(res))
		return std::move(std::get<Blob>(res));
	return std::nullopt;
}

std::optional<Commit> Repo::commitLookup(const git_oid &oid) const
{
	return MakeGit<Commit>(git_commit_lookup, repo(), &oid);
}

std::optional<Commit> Repo::commitCreate(const Signature &author, const Signature &committer,
					 const std::string &msg, const Tree &tree,
					 const std::vector<const Commit *> &parents) const
{
	git_oid oid;
	std::vector<const git_commit *> parentPtrs;

	for (const auto &p : parents)
		parentPtrs.push_back(p->commit());

	if (git_commit_create(&oid, repo(), "HEAD", author, committer, "UTF-8", msg.c_str(),
			      tree, parentPtrs.size(), parentPtrs.data()))
		return std::nullopt;
	return commitLookup(oid);
}

std::optional<Commit> Repo::commitCreateCheckout(const Signature &author,
						 const Signature &committer,
						 const std::string &msg, const Tree &tree,
						 unsigned int strategy,
						 const std::vector<const Commit *> &parents) const
{
	auto commit = commitCreate(author, committer, msg, tree, parents);
	if (!commit)
		return std::nullopt;
	if (checkoutTree(tree, strategy))
		return std::nullopt;
	return commit;
}

std::optional<Commit> Repo::commitHead() const noexcept
{
	return commitRevparseSingle("HEAD");
}

std::optional<Commit> Repo::commitRevparseSingle(const std::string &rev) const
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Commit>(res))
		return std::move(std::get<Commit>(res));
	return std::nullopt;
}

std::variant<Blob, Commit, Tag, Tree, std::monostate> Repo::revparseSingle(const std::string &rev) const
{
	git_object *obj;
	if (git_revparse_single(&obj, repo(), rev.c_str()))
		return std::monostate{};

	switch (git_object_type(obj)) {
	case GIT_OBJECT_BLOB:
		return Blob(reinterpret_cast<git_blob *>(obj));
	case GIT_OBJECT_COMMIT:
		return Commit(reinterpret_cast<git_commit *>(obj));
	case GIT_OBJECT_TAG:
		return Tag(reinterpret_cast<git_tag *>(obj));
	case GIT_OBJECT_TREE:
		return Tree(reinterpret_cast<git_tree *>(obj));
	default:
		git_object_free(obj);
		return std::monostate{};
	}
}

std::optional<Tree> Repo::treeLookup(const git_oid &oid) const
{
	return MakeGit<Tree>(git_tree_lookup, repo(), &oid);
}

std::optional<Tree> Repo::treeLookup(const TreeEntry &tentry) const
{
	return treeLookup(*tentry.id());
}

std::optional<Tree> Repo::treeRevparseSingle(const std::string &rev) const
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Tree>(res))
		return std::move(std::get<Tree>(res));
	return std::nullopt;
}

std::optional<Index> Repo::index() const
{
	return MakeGit<Index>(git_repository_index, repo());
}

std::optional<Remote> Repo::remoteCreate(const std::string &name, const std::string &url) const
{
	return MakeGit<Remote>(git_remote_create, repo(), name.c_str(), url.c_str());
}

std::optional<Remote> Repo::remoteLookup(const std::string &name) const
{
	return MakeGit<Remote>(git_remote_lookup, repo(), name.c_str());
}

std::optional<RevWalk> Repo::revWalkCreate() const
{
	return MakeGit<RevWalk>(git_revwalk_new, repo());
}

std::optional<Tag> Repo::tagCreate(const std::string &tagName, const Object &target,
				   const Signature &tagger, const std::string &message,
				   bool force) const
{
	git_oid oid;
	if (git_tag_create(&oid, repo(), tagName.c_str(), target.object(), tagger, message.c_str(),
			   force))
		return std::nullopt;
	return tagLookup(oid);
}

std::optional<Tag> Repo::tagLookup(const git_oid &oid) const
{
	return MakeGit<Tag>(git_tag_lookup, repo(), &oid);
}

std::optional<Tag> Repo::tagLookup(const TreeEntry &tentry) const
{
	return tagLookup(*tentry.id());
}

std::optional<Tag> Repo::tagRevparseSingle(const std::string &rev) const
{
	auto res = revparseSingle(rev);
	if (std::holds_alternative<Tag>(res))
		return std::move(std::get<Tag>(res));
	return std::nullopt;
}

std::optional<Reference> Repo::refLookup(const std::string &name) const
{
	return MakeGit<Reference>(git_reference_lookup, repo(), name.c_str());
}

std::optional<Reference> Repo::refDWIM(const std::string &name) const
{
	return MakeGit<Reference>(git_reference_dwim, repo(), name.c_str());
}

std::optional<Reference> Repo::refCreateDirect(const std::string &name, const git_oid &oid,
					       bool force) const {
	return MakeGit<Reference>(git_reference_create, repo(), name.c_str(), &oid, force, nullptr);
}

std::optional<Reference> Repo::refCreateSymbolic(const std::string &name, const std::string &target,
						 bool force) const
{
	return MakeGit<Reference>(git_reference_symbolic_create, repo(), name.c_str(),
				  target.c_str(), force, nullptr);
}

std::optional<TreeBuilder> Repo::treeBuilderCreate(const Tree *source) const
{
	return MakeGit<TreeBuilder>(git_treebuilder_new, repo(), source ? source->tree() : nullptr);
}

std::optional<Repo> Repo::clone(const std::filesystem::path &path, const std::string &url,
				const std::string &branch, const unsigned int &depth, bool tags)
{
	MyFetchCallbacks fc;
	git_clone_options opts GIT_CLONE_OPTIONS_INIT;
	opts.checkout_branch = branch.empty() ? nullptr : branch.c_str();
	opts.fetch_opts.depth = depth;
	if (!tags)
		opts.fetch_opts.download_tags = GIT_REMOTE_DOWNLOAD_TAGS_NONE;
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

int Repo::checkout(const std::string &branch) const
{
	auto ref = refLookup(branch);
	if (!ref)
		return -1;

	return checkout(*ref);
}

int Repo::checkout(const Reference &ref) const
{
	auto c = commitLookup(*ref.target());
	if (!c)
		return -1;

	auto tree = c->tree();
	if (!tree)
		return -1;

	auto ret = checkoutTree(*tree);
	if (ret)
		return ret;

	ret = git_repository_set_head(repo(), ref.name().c_str());
	if (ret)
		return ret;

	return 0;
}
