// SPDX-License-Identifier: GPL-2.0-only

#include <cstring>
#include <sstream>

#include <spawn.h>
#include <unistd.h>

#include <sys/wait.h>

#include "helpers/Process.h"

using namespace SlHelpers;

Process::~Process()
{
	if (m_pid >= 0)
		waitForFinished();
}

int Process::spawn(const std::filesystem::path &program, const std::vector<std::string> &args,
		   std::string *out)
{
	if (m_pid >= 0) {
		setError(BusyError, "The previous pid still active");
		return -1;
	}

	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(program.c_str()));
	for (const auto &a : args)
		argv.push_back(const_cast<char *>(a.c_str()));
	argv.push_back(nullptr);

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	if (out) {
		if (pipe(m_pipe)) {
			setError(PipeError, strerror(errno));
			return -1;
		}
		posix_spawn_file_actions_addclose(&actions, STDOUT_FILENO);
		posix_spawn_file_actions_adddup2(&actions, m_pipe[1], STDOUT_FILENO);
		posix_spawn_file_actions_addclose(&actions, m_pipe[0]);
		posix_spawn_file_actions_addclose(&actions, m_pipe[1]);
	}

	auto ret = ::posix_spawn(&m_pid, program.c_str(), &actions, nullptr, argv.data(), environ);
	closePipe(1);
	posix_spawn_file_actions_destroy(&actions);
	if (ret) {
		closePipe(0);
		setError(SpawnError, strerror(errno));
		return -1;
	}

	return 0;
}

int Process::waitForFinished()
{
	int stat;
	auto ret = waitpid(m_pid, &stat, 0);
	m_pid = -1;
	closePipe(0);
	closePipe(1);
	if (ret < 0) {
		setError(WaitPidError, strerror(errno));
		return -1;
	}

	if (WIFEXITED(stat)) {
		m_exitStatus = WEXITSTATUS(stat);
		return 0;
	}

	if (WIFSIGNALED(stat)) {
		m_signalled = true;
		return 0;
	}

	return 0;
}

int Process::readAll(std::string &out)
{
	char buf[128];
	std::stringstream ss;
	while (true) {
		auto r = read(m_pipe[0], buf, sizeof(buf));
		if (r < 0) {
			setError(ReadError, strerror(errno));
			return -1;
		}
		if (!r)
			break;
		ss.write(buf, r);
	}
	closePipe(0);
	out = ss.str();

	return 0;
}

int Process::run(const std::filesystem::path &program, const std::vector<std::string> &args,
		 std::string *out)
{
	if (spawn(program, args, out))
		return -1;

	auto ret = out ? readAll(*out) : 0;

	return waitForFinished() ? : ret;
}

void Process::closePipe(const unsigned int &p)
{
	if (m_pipe[p] < 0)
		return;

	close(m_pipe[p]);
	m_pipe[p] = -1;
}
