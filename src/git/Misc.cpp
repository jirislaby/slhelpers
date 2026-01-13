// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Misc.h"
#include "git/Repo.h"
#include "git/Tag.h"
#include "git/Tree.h"

using namespace SlGit;

template<>
void SlHelpers::Deleter<git_reference>::operator()(git_reference *ref) const
{
	git_reference_free(ref);
}

template<>
void SlHelpers::Deleter<git_revwalk>::operator()(git_revwalk *revWalk) const
{
	git_revwalk_free(revWalk);
}

template<>
void SlHelpers::Deleter<git_signature>::operator()(git_signature *sig) const
{
	git_signature_free(sig);
}

std::optional<Reference> Reference::resolve() const noexcept
{
	git_reference *out;
	if (git_reference_resolve(&out, ref()))
		return std::nullopt;
	return Reference(out);
}

bool RevWalk::push(const std::string &id) const noexcept
{
	const auto obj = m_repo.revparseSingle(id);
	const git_oid *oid;
	if (auto commit = std::get_if<Commit>(&obj))
		oid = commit->id();
	else if (auto tag = std::get_if<Tag>(&obj))
		oid = tag->id();
	else
		return false;

	return !Repo::setLastError(git_revwalk_push(revWalk(), oid));
}

bool RevWalk::hide(const std::string &id) const noexcept
{
	const auto obj = m_repo.revparseSingle(id);
	const git_oid *oid;
	if (auto commit = std::get_if<Commit>(&obj))
		oid = commit->id();
	else if (auto tag = std::get_if<Tag>(&obj))
		oid = tag->id();
	else
		return false;

	return !Repo::setLastError(git_revwalk_hide(revWalk(), oid));
}

std::optional<Commit> RevWalk::next() const noexcept
{
	git_oid oid;
	if (Repo::setLastError(git_revwalk_next(&oid, revWalk())))
		return std::nullopt;
	return m_repo.commitLookup(oid);
}

std::optional<Signature> Signature::now(const std::string &name, const std::string &email) {
	git_signature *sig;
	if (Repo::setLastError(git_signature_now(&sig, name.c_str(), email.c_str())))
		return std::nullopt;
	return Signature(sig);
}
