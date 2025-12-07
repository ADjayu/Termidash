#include "core/PromptEngine.hpp"
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <lmcons.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

namespace termidash {

PromptEngine& PromptEngine::instance() {
    static PromptEngine instance;
    return instance;
}

PromptEngine::PromptEngine() 
    : ps1_("\\u@\\h:\\w\\$ "), defaultPrompt_("termidash> ") {
}

void PromptEngine::setPS1(const std::string& format) {
    ps1_ = format;
}

const std::string& PromptEngine::getPS1() const {
    return ps1_;
}

void PromptEngine::setDefaultPrompt(const std::string& prompt) {
    defaultPrompt_ = prompt;
}

std::string PromptEngine::getUsername() const {
#ifdef _WIN32
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserNameA(username, &size)) {
        return std::string(username);
    }
    // Fallback to environment variable
    const char* user = std::getenv("USERNAME");
    return user ? user : "user";
#else
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        return std::string(pw->pw_name);
    }
    const char* user = std::getenv("USER");
    return user ? user : "user";
#endif
}

std::string PromptEngine::getHostname(bool full) const {
#ifdef _WIN32
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerNameA(hostname, &size)) {
        std::string result(hostname);
        if (!full) {
            size_t dotPos = result.find('.');
            if (dotPos != std::string::npos) {
                result = result.substr(0, dotPos);
            }
        }
        return result;
    }
    const char* name = std::getenv("COMPUTERNAME");
    return name ? name : "localhost";
#else
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, HOST_NAME_MAX) == 0) {
        std::string result(hostname);
        if (!full) {
            size_t dotPos = result.find('.');
            if (dotPos != std::string::npos) {
                result = result.substr(0, dotPos);
            }
        }
        return result;
    }
    return "localhost";
#endif
}

std::string PromptEngine::getCurrentDirectory(bool basename) const {
    try {
        fs::path cwd = fs::current_path();
        std::string result = cwd.string();
        
        // Replace home directory with ~
#ifdef _WIN32
        const char* home = std::getenv("USERPROFILE");
#else
        const char* home = std::getenv("HOME");
#endif
        if (home) {
            std::string homePath(home);
            // Normalize path separators for comparison
            std::replace(homePath.begin(), homePath.end(), '\\', '/');
            std::string normalizedResult = result;
            std::replace(normalizedResult.begin(), normalizedResult.end(), '\\', '/');
            
            if (normalizedResult == homePath) {
                result = "~";
            } else if (normalizedResult.find(homePath + "/") == 0) {
                result = "~" + normalizedResult.substr(homePath.length());
            }
        }
        
        if (basename) {
            fs::path p(result);
            return p.filename().string().empty() ? result : p.filename().string();
        }
        
        return result;
    } catch (...) {
        return "?";
    }
}

bool PromptEngine::isAdmin() const {
#ifdef _WIN32
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin != FALSE;
#else
    return getuid() == 0;
#endif
}

std::string PromptEngine::getTime(bool format24h) const {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    
    std::ostringstream oss;
    if (format24h) {
        oss << std::setfill('0') << std::setw(2) << local->tm_hour << ":"
            << std::setfill('0') << std::setw(2) << local->tm_min << ":"
            << std::setfill('0') << std::setw(2) << local->tm_sec;
    } else {
        int hour = local->tm_hour % 12;
        if (hour == 0) hour = 12;
        oss << std::setfill('0') << std::setw(2) << hour << ":"
            << std::setfill('0') << std::setw(2) << local->tm_min << ":"
            << std::setfill('0') << std::setw(2) << local->tm_sec
            << (local->tm_hour >= 12 ? " PM" : " AM");
    }
    return oss.str();
}

std::string PromptEngine::getDate() const {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    
    static const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    std::ostringstream oss;
    oss << days[local->tm_wday] << " " << months[local->tm_mon] << " "
        << std::setfill('0') << std::setw(2) << local->tm_mday;
    return oss.str();
}

std::string PromptEngine::expandEscape(const std::string& format, size_t pos, size_t& consumed) const {
    consumed = 2; // Default: consume backslash + one character
    
    if (pos + 1 >= format.size()) {
        consumed = 1;
        return "\\";
    }
    
    char c = format[pos + 1];
    
    switch (c) {
        case 'u': return getUsername();
        case 'h': return getHostname(false);
        case 'H': return getHostname(true);
        case 'w': return getCurrentDirectory(false);
        case 'W': return getCurrentDirectory(true);
        case '$': return isAdmin() ? "#" : "$";
        case 't': return getTime(true);
        case 'T': return getTime(false);
        case 'd': return getDate();
        case 'n': return "\n";
        case 'r': return "\r";
        case '\\': return "\\";
        case 'e': return "\033";
        case '[': return ""; // Begin non-printing sequence (for color codes)
        case ']': return ""; // End non-printing sequence
        default:
            // Unknown escape - return as-is
            return std::string("\\") + c;
    }
}

std::string PromptEngine::render() const {
    const std::string& format = ps1_.empty() ? defaultPrompt_ : ps1_;
    std::string result;
    result.reserve(format.size() * 2); // Pre-allocate
    
    for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] == '\\') {
            size_t consumed;
            result += expandEscape(format, i, consumed);
            i += consumed - 1; // -1 because loop will ++i
        } else {
            result += format[i];
        }
    }
    
    return result;
}

} // namespace termidash
