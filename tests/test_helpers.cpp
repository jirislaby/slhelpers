// SPDX-License-Identifier: GPL-2.0-only

#include <cassert>

#include "helpers/Color.h"
#include "helpers/Exception.h"
#include "helpers/HomeDir.h"
#include "helpers/LastError.h"
#include "helpers/Process.h"
#include "helpers/PtrStore.h"
#include "helpers/PushD.h"
#include "helpers/SSH.h"
#include "helpers/Views.h"

#include "helpers.h"

using namespace SlHelpers;

namespace SlHelpers {

void testColor()
{
	using Clr = Color;

	{
		std::ostringstream oss;
		std::ostringstream oss2;

		static constexpr const std::string_view texts[] = {
			"Test print",
			"Test print NoNL",
			" .. continued",
		};
		oss2 << Clr::seqBegin << Clr::DEFAULT << 'm' << texts[0] << Color::seqEnd << '\n' <<
			Clr::seqBegin << Clr::DEFAULT << 'm' << texts[1] << Color::seqEnd <<
			Clr::seqBegin << Clr::DEFAULT << 'm' << texts[2] << Color::seqEnd << '\n';
		Clr(oss) << texts[0];
		Clr(oss) << texts[1] << Color::NoNL;
		Clr(oss) << texts[2];
		std::cerr << oss.str();
		assert(oss.str() == oss2.str());
	}
	{
		std::ostringstream oss;
		std::ostringstream oss2;

		static constexpr const std::string_view text("Test print in RED");
		oss2 << Clr::seqBegin << Clr::RED << 'm' << text << Color::seqEnd << '\n';
		Clr(oss, Clr::RED) << text;
		std::cerr << oss.str();
		assert(oss.str() == oss2.str());
	}
	{
		std::ostringstream oss;
		std::ostringstream oss2;

		static constexpr const std::string_view text("Test print in RGB(0, 255, 255)");
		oss2 << Clr::seqBegin << Clr::COL256 << ";2;0;255;255m" << text <<
			Color::seqEnd << '\n';
		Clr(oss, 0, 255, 255) << text;
		std::cerr << oss.str();
		assert(oss.str() == oss2.str());
	}
}

} // namespace

namespace SlSSH {

void testKeys()
{
	assert(Keys::handleTokens("", "") == "");
	assert(Keys::handleTokens("", "~") == "~");
	assert(Keys::handleTokens("", "%%") == "%");
	assert(Keys::handleTokens("", "some%") == "some%");
	assert(Keys::handleTokens("", "some%%") == "some%");
	assert(Keys::handleTokens("some_host", "some/%h/file") == "some/some_host/file");
	{
		THelpers::RestoreEnv home("HOME");
		setenv("HOME", "/tmp/", true);
		assert(Keys::handleTokens("", "~/some") == "/tmp/some");
		assert(Keys::handleTokens("", "s%dome") == "s/tmp/ome");
	}
}

} // namespace

namespace {

void testException()
{
	static constinit std::string_view text("this is exception: ");
	static constinit std::string_view text2("some reason");
	std::string composed(text);
	composed.append(text2);

	auto e = RuntimeException(text);
	assert(e.str() == text);
	e << text2;
	assert(e.str() == composed);

	{
		bool caught = false;
		try {
			e.raise();
		} catch (std::runtime_error &e) {
			assert(e.what() == composed);
			caught = true;
		}
		assert(caught);
	}
	{
		bool caught = false;
		try {
			e << raise;
		} catch (std::runtime_error &e) {
			assert(e.what() == composed);
			caught = true;
		}
		assert(caught);
	}
	{
		bool caught = false;
		try {
			throw e.getRE();
		} catch (std::runtime_error &e) {
			assert(e.what() == composed);
			caught = true;
		}
		assert(caught);
	}
	{
		bool caught = false;
		try {
			RuntimeException(text) << text2 << raise;
		} catch (std::runtime_error &e) {
			assert(e.what() == composed);
			caught = true;
		}
		assert(caught);
	}
}

void testHomeDir()
{
	THelpers::RestoreEnv xdg("XDG_CACHE_HOME");
	THelpers::RestoreEnv home("HOME");
	if (home)
		assert(HomeDir::get() == home.value());

	setenv("XDG_CACHE_HOME", "/xdg_cache", true);
	setenv("HOME", "/tmp/", true);
	assert(HomeDir::getCacheDir() == "/xdg_cache");
	unsetenv("XDG_CACHE_HOME");
	assert(HomeDir::getCacheDir() == "/tmp/.cache");

	const auto tmpDir = THelpers::getTmpDir();
	setenv("HOME", tmpDir.c_str(), true);
	assert(HomeDir::get() == tmpDir);

	const auto cacheDir = HomeDir::getCacheDir();
	assert(cacheDir == tmpDir / ".cache");

	auto createdCacheDir = HomeDir::createCacheDir("1");
	assert(createdCacheDir == cacheDir / "1");
	assert(std::filesystem::exists(createdCacheDir));
	std::filesystem::remove_all(tmpDir);
}

void testLastError()
{
	static const constinit std::string_view text("text");
	static const constinit std::string_view moreText("more");
	static const auto merged(std::string(text).append(moreText));

	{
		LastErrorStream e;

		assert(e.lastError().empty());
		e.reset() << text;
		assert(e.lastError() == text);
		e << moreText;
		assert(e.lastError() == merged);
	}
	{
		LastErrorStr<int> e;
		assert(e.lastError().empty());
		assert(e.get<0>() == 0);
		e.set<0>(1000);
		assert(e.get<0>() == 1000);
		e.reset().setError(text);
		assert(e.lastError() == text);
		assert(e.get<0>() == 0);
	}
	{
		LastErrorStr<int, std::string> e;
		assert(e.get<1>().empty());
		e.set<1>(text);
		assert(e.get<1>() == text);
		e.get<1>().append(moreText);
		assert(e.get<1>() == merged);
		e.reset();
		assert(e.get<1>().empty());
	}
}

void testProcess(const std::filesystem::path &crash)
{
	Process p;

	assert(p.run("/usr/bin/true"));
	assert(!p.signalled());
	assert(!p.exitStatus());

	assert(p.run("/usr/bin/false"));
	assert(!p.signalled());
	assert(p.exitStatus());

	assert(!p.run("/does_not_exist/bin"));
	assert(p.lastErrorNo() == Process::SpawnError);

	std::string s;
	assert(p.run("/usr/bin/echo", { "-e", "one", "two\\n\\tthree" }, &s));
	assert(!p.signalled());
	assert(!p.exitStatus());
	assert(s == "one two\n\tthree\n");

	assert(p.run(crash));
	assert(p.signalled());
}

void testPtrStore()
{
	static constinit std::string_view text{"1"};
	static int refcount;

	using CharPtrStore = SlHelpers::PtrStore<char, decltype([](char *p) {
							if (!p)
								return;
							assert(p == text.data());
							refcount--;
						})>;
	static_assert(!std::is_copy_constructible<CharPtrStore>());
	static_assert(!std::is_copy_assignable<CharPtrStore>());
	static_assert(std::is_move_constructible<CharPtrStore>());
	static_assert(std::is_move_assignable<CharPtrStore>());

	{
		CharPtrStore ptr;
		*ptr.ptr() = const_cast<char *>(text.data());
		refcount++;
		assert(ptr.get() == text.data());
		std::ostringstream oss;
		oss << ptr;
		assert(oss.str() == text);
	}
	assert(refcount == 0);

	{
		CharPtrStore ptr(const_cast<char *>(text.data()));
		assert(ptr.get() == text.data());
		ptr.release();
		assert(ptr.get() == nullptr);
		assert(refcount == 0);

		ptr.reset(const_cast<char *>(text.data()));
		refcount++;
		assert(ptr.get() == text.data());
		ptr.reset();
		assert(ptr.get() == nullptr);
		assert(refcount == 0);

		ptr.reset(const_cast<char *>(text.data()));
		refcount++;
		CharPtrStore ptr2;
		ptr2 = std::move(ptr);
		assert(ptr2);
		assert(ptr2.str() == text);
	}
	assert(refcount == 0);
}

void testPushD()
{
	const auto orig = std::filesystem::current_path();
	{
		std::error_code ec;
		PushD p1("/", ec);
		assert(!ec);
		assert(std::filesystem::current_path() == "/");
		PushD p2("/tmp", ec);
		assert(!ec);
		assert(std::filesystem::current_path() == "/tmp");
	}
	assert(std::filesystem::current_path() == orig);
}

void testViews()
{
	static const std::vector<std::string_view> vec { "a", "b", "c", "d" };

	{
		unsigned i = 0;
		for (const auto &[fi, se]: vec | Views::pairwise) {
			assert(fi == vec[i]);
			assert(se == vec[i + 1]);
			i++;
		}
		assert(i < vec.size());
	}
	{
		auto x = Views::pairwise(vec);
		auto it = x.begin();
		assert(it.operator*().first == vec[0]);
		assert(it.operator*().second == vec[1]);
		++it;
		assert(it.operator*().first == vec[1]);
		assert(it.operator*().second == vec[2]);
	}
}

} // namespace

int main(int argc, char **argv)
{
	assert(argc > 1);
	testColor();
	testException();
	testHomeDir();
	testLastError();
	testProcess(argv[1]);
	testPtrStore();
	testPushD();
	testViews();
	SlSSH::testKeys();

	return 0;
}

