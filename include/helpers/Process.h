// SPDX-License-Identifier: GPL-2.0-only

#ifndef SLHELPERS_PROCESS_H
#define SLHELPERS_PROCESS_H

#include <filesystem>
#include <string>
#include <vector>

#include <sys/types.h>

namespace SlHelpers {

class Process {
public:
	enum Error {
		UnknownError,
		BusyError,
		PipeError,
		SpawnError,
		WaitPidError,
		ReadError,
		WriteError,
	};
	Process() : m_pid(-1), m_pipe{-1, -1}, m_signalled(false), m_exitStatus(0), m_lastError(""),
		m_lastErrorNo(UnknownError) {}
	~Process();

	/* spawn + read (if 'out' non-null) + wait */
	int run(const std::filesystem::path &program, const std::vector<std::string> &args = {},
		std::string *out = nullptr);

	int spawn(const std::filesystem::path &program, const std::vector<std::string> &args = {},
		  std::string *out = nullptr);
	int readAll(std::string &out);
	int waitForFinished();

	pid_t pid() const { return m_pid; }
	bool signalled() const { return m_signalled; }
	unsigned int exitStatus() const { return m_exitStatus; }

	const std::string &lastError() const { return m_lastError; }
	Error lastErrorNo() const { return m_lastErrorNo; }
private:
	void closePipe(const unsigned int &p);
	void setError(const Error &errNo, const std::string &error) {
		m_lastErrorNo = errNo;
		m_lastError = error;
	}

	pid_t m_pid;
	int m_pipe[2];
	bool m_signalled;
	unsigned int m_exitStatus;
	std::string m_lastError;
	Error m_lastErrorNo;
};

}

#endif
