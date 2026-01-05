// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <sys/types.h>

namespace SlHelpers {

/**
 * @brief Creates a process and executes a program
 */
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

	/**
	 * @brief Spawns a process, executes \p program with \p args, and waits for its termination
	 * @param program Program to execute
	 * @param args Arguments passed to the program (do not add \p program itself)
	 * @param out stdout of the \p program
	 * @return zero on success
	 *
	 * If \p out is non-NULL, the stdout of the \p program is filled in there.
	 * run() combines spawn(), readAll(), and waitForFinished().
	 */
	bool run(const std::filesystem::path &program, const std::vector<std::string> &args = {},
		 std::string *out = nullptr);

	/**
	 * @brief Forks a process and executes \p program with \p args
	 * @param program Program to run
	 * @param args Arguments to pass to \p program
	 * @param captureStdout Stdout of the program. If nullptr, output is discarded
	 * @return true on success.
	 */
	bool spawn(const std::filesystem::path &program, const std::vector<std::string> &args = {},
		   bool captureStdout = false);

	/**
	 * @brief Read everything from process' stdout
	 * @param out Where to read to
	 * @return true on success.
	 */
	bool readAll(std::string &out);

	/**
	 * @brief Wait until the underlying process is finished (or dead)
	 * @return true on success.
	 */
	bool waitForFinished();

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
