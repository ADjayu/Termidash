#include "common/PlatformUtils.hpp"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "Shell32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#endif

#include <filesystem>

namespace PlatformUtils {

    std::string getHomeDirectory() {
#ifdef _WIN32
        char path[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path) == S_OK) {
            return std::string(path);
        }
        return "";
#else
        const char* home = getenv("HOME");
        if (home) return std::string(home);
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) return std::string(pwd->pw_dir);
        return "";
#endif
    }

    std::string getHistoryFilePath() {
        std::string home = getHomeDirectory();
        if (home.empty()) return ".termidash_history";
#ifdef _WIN32
        return home + "\\.termidash_history";
#else
        return home + "/.termidash_history";
#endif
    }

    long openFileForRead(const std::string& path) {
#ifdef _WIN32
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return -1;
        return (long)hFile;
#else
        int fd = open(path.c_str(), O_RDONLY);
        return (long)fd;
#endif
    }

    long openFileForWrite(const std::string& path, bool append) {
#ifdef _WIN32
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        DWORD creation = append ? OPEN_ALWAYS : CREATE_ALWAYS;
        HANDLE hFile = CreateFileA(path.c_str(), append ? FILE_APPEND_DATA : GENERIC_WRITE, FILE_SHARE_READ, &sa, creation, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return -1;
        if (append) {
            SetFilePointer(hFile, 0, NULL, FILE_END);
        }
        return (long)hFile;
#else
        int flags = O_WRONLY | O_CREAT;
        if (append) flags |= O_APPEND;
        else flags |= O_TRUNC;
        int fd = open(path.c_str(), flags, 0644);
        return (long)fd;
#endif
    }

    void closeFile(long handle) {
        if (handle == -1) return;
#ifdef _WIN32
        CloseHandle((HANDLE)handle);
#else
        close((int)handle);
#endif
    }

    std::string getEnv(const std::string& name) {
#ifdef _WIN32
        char* buf = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&buf, &sz, name.c_str()) == 0 && buf != nullptr) {
            std::string val(buf);
            free(buf);
            return val;
        }
        return "";
#else
        const char* val = getenv(name.c_str());
        return val ? std::string(val) : "";
#endif
    }

    char getPathSeparator() {
#ifdef _WIN32
        return ';';
#else
        return ':';
#endif
    }

}
