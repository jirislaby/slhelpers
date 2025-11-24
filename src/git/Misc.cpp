// SPDX-License-Identifier: GPL-2.0-only

#include <git2.h>

#include "git/Commit.h"
#include "git/Misc.h"
#include "git/Repo.h"

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

std::optional<Commit> RevWalk::next() const noexcept
{
	git_oid oid;
	if (git_revwalk_next(&oid, revWalk()))
		return std::nullopt;
	return m_repo.commitLookup(oid);
}

std::optional<Signature> Signature::now(const std::string &name, const std::string &email) {
	git_signature *sig;
	if (git_signature_now(&sig, name.c_str(), email.c_str()))
		return std::nullopt;
	return Signature(sig);
}
