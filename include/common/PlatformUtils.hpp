#pragma once
#include <string>

namespace PlatformUtils {
    std::string getHistoryFilePath();
    std::string getHomeDirectory();
    
    // File I/O for redirection
    // Returns handle/fd cast to long, or -1 on failure
    long openFileForRead(const std::string& path);
    long openFileForWrite(const std::string& path, bool append);
    void closeFile(long handle);

    // Environment and Path
    std::string getEnv(const std::string& name);
    char getPathSeparator();
}
