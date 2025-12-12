#pragma once
#include <string>

namespace PlatformUtils {
std::string getHistoryFilePath();
std::string getHomeDirectory();

// File I/O for redirection
// Returns handle/fd cast to long, or -1 on failure
long openFileForRead(const std::string &path);
long openFileForWrite(const std::string &path, bool append);
void closeFile(long handle);

// Environment and Path
std::string getEnv(const std::string &name);
char getPathSeparator();

// Path normalization utilities
// Converts backslashes to forward slashes for internal use
std::string normalizePath(const std::string &path);

// Converts to OS-native path separator for API calls
std::string toNativePath(const std::string &path);

// Line ending normalization - strips \r from CRLF
std::string normalizeLineEndings(const std::string &text);

// Get the directory separator character for current platform
char getDirSeparator();
} // namespace PlatformUtils
