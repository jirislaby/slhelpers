// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <chrono>
#include <filesystem>
#include <optional>
#include <ostream>
#include <string>

typedef void CURL;

namespace SlCurl {

/**
 * @brief Wrapper class for libcurl
 */
class LibCurl {
public:
	/**
	 * @brief Construct LibCurl
	 *
	 * Either make a LibCurl and use non-static methods; or use the static methods.
	 */
	LibCurl();
	~LibCurl();

	/**
	 * @brief Download \p url to \p stream
	 * @param url URL to download
	 * @param stream Stream to store to
	 * @param HTTPErrorCode HTTP error code returned from the server (or nullptr)
	 * @return true for success.
	 */
	bool downloadToStream(const std::string &url, const std::ostream &stream,
			      unsigned *HTTPErrorCode = nullptr);

	/**
	 * @brief Download \p url to \p file
	 * @param url URL to download
	 * @param file Where to store the downloaded content to
	 * @param HTTPErrorCode HTTP error code returned from the server (or nullptr)
	 * @return true for success.
	 */
	bool downloadToFile(const std::string &url, const std::filesystem::path &file,
			    unsigned *HTTPErrorCode = nullptr);


	/**
	 * @brief Download \p url to a string
	 * @param url URL to download
	 * @param HTTPErrorCode HTTP error code returned from the server (or nullptr)
	 * @return Downloaded content or nullopt on failure.
	 */
	std::optional<std::string> download(const std::string &url,
					    unsigned *HTTPErrorCode = nullptr);

	/**
	 * @brief Download \p url to a string (and create LibCurl temporarily)
	 * @param url URL to download
	 * @param HTTPErrorCode HTTP error code returned from the server (or nullptr)
	 * @return Downloaded content or nullopt on failure.
	 */
	static std::optional<std::string> singleDownload(const std::string &url,
							 unsigned *HTTPErrorCode = nullptr);

	/**
	 * @brief Download \p url to \p file (and create LibCurl temporarily)
	 * @param url URL to download
	 * @param file Where to store the downloaded content to
	 * @param HTTPErrorCode HTTP error code returned from the server (or nullptr)
	 * @return true for success.
	 */
	static bool singleDownloadToFile(const std::string &url,
					 const std::filesystem::path &file,
					 unsigned *HTTPErrorCode = nullptr);

	/**
	 * @brief Check if \p filePath is old enough that it should be downloaded
	 * @param filePath The file to check
	 * @param fileAlreadyExists Stored true if the file exists already
	 * @param forceRefresh Download in any case.
	 * @param hours After how many hours this file expires (and is downloaded again)
	 * @return True if a download should be performed.
	 */
	static bool isDownloadNeeded(const std::filesystem::path &filePath, bool &fileAlreadyExists,
				     bool forceRefresh, const std::chrono::hours &hours);

	/**
	 * @brief Fetch \p url and store into \p filePath
	 * @param filePath File to store to
	 * @param url The URL to fetch
	 * @param forceRefresh Fetch in any case (ignore \p hours)
	 * @param ignoreErrors Errors are ignored
	 * @param hours After how many hours this file expires (and is downloaded again)
	 * @return Path to the fetched file or nullopt in case of error.
	 */
	static std::filesystem::path fetchFileIfNeeded(const std::filesystem::path &filePath,
						       const std::string &url,
						       bool forceRefresh, bool ignoreErrors,
						       const std::chrono::hours &hours);

	/// @brief Return the last error string if some
	static const std::string &lastError() { return m_lastError; }
private:
	CURL *handle;
	static thread_local std::string m_lastError;
};

}
