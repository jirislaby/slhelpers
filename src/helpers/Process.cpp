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

bool Process::spawn(const std::filesystem::path &program, const std::vector<std::string> &args,
		    bool captureStdout)
{
	if (m_pid >= 0) {
		setError(BusyError, "The previous pid still active");
		return false;
	}

	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(program.c_str()));
	for (const auto &a : args)
		argv.push_back(const_cast<char *>(a.c_str()));
	argv.push_back(nullptr);

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	if (captureStdout) {
		if (pipe(m_pipe)) {
			setError(PipeError, strerror(errno));
			return false;
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
		return false;
	}

	return true;
}

bool Process::waitForFinished()
{
	int stat;
	auto ret = waitpid(m_pid, &stat, 0);
	m_pid = -1;
	closePipe(0);
	closePipe(1);
	if (ret < 0) {
		setError(WaitPidError, strerror(errno));
		return false;
	}

	if (WIFEXITED(stat)) {
		m_exitStatus = WEXITSTATUS(stat);
		if (m_exitStatus == 127) { // 127 defined in man
			setError(SpawnError, "pre-exec or execve() failure");
			return false;
		}
		return true;
	}

	if (WIFSIGNALED(stat)) {
		m_signalled = true;
		return true;
	}

	return true;
}

bool Process::kill(int sig)
{
	if (::kill(m_pid, sig) < 0) {
		setError(UnknownError, strerror(errno));
		return false;
	}

	return true;
}

bool Process::readAll(std::string &out)
{
	char buf[128];
	std::ostringstream ss;
	while (true) {
		auto r = read(m_pipe[0], buf, sizeof(buf));
		if (r < 0) {
			setError(ReadError, strerror(errno));
			return false;
		}
		if (!r)
			break;
		ss.write(buf, r);
	}
	closePipe(0);
	out = ss.str();

	return true;
}

bool Process::run(const std::filesystem::path &program, const std::vector<std::string> &args,
		 std::string *out)
{
	if (!spawn(program, args, out))
		return false;

	auto ret = true;
	if (out)
		ret = readAll(*out);

	return waitForFinished() && ret;
}

void Process::closePipe(const unsigned int &p)
{
	if (m_pipe[p] < 0)
		return;

	close(m_pipe[p]);
	m_pipe[p] = -1;
}
