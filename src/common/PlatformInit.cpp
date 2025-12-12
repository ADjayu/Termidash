#include "common/PlatformInit.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <fcntl.h>
#include <io.h>
#include <windows.h>

#else
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace termidash {

// Store original console settings for restoration on Windows
#ifdef _WIN32
static UINT originalInputCP = 0;
static UINT originalOutputCP = 0;
static DWORD originalOutputMode = 0;
static HANDLE hStdOut = INVALID_HANDLE_VALUE;
static bool platformInitialized = false;
#endif

PlatformInitResult initializePlatform() {
  PlatformInitResult result;

#ifdef _WIN32
  // Store original code pages for cleanup
  originalInputCP = GetConsoleCP();
  originalOutputCP = GetConsoleOutputCP();

  // Set console to UTF-8 encoding (code page 65001)
  if (SetConsoleCP(65001) && SetConsoleOutputCP(65001)) {
    result.utf8Enabled = true;
  } else {
    result.errorMessage = "Failed to set UTF-8 console encoding";
  }

  // Enable Virtual Terminal Processing for ANSI escape sequences
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hStdOut != INVALID_HANDLE_VALUE) {
    // Get current output mode
    if (GetConsoleMode(hStdOut, &originalOutputMode)) {
      // Enable VT100 mode
      DWORD newMode = originalOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      if (SetConsoleMode(hStdOut, newMode)) {
        result.vt100Enabled = true;
      } else {
        // VT100 failed but might work on Windows 10+
        result.errorMessage += (result.errorMessage.empty() ? "" : "; ");
        result.errorMessage +=
            "VT100 mode not available (Windows 10+ required)";
      }
    }
  }

  // Also enable VT100 for input (for escape sequence handling)
  HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdIn != INVALID_HANDLE_VALUE) {
    DWORD inputMode;
    if (GetConsoleMode(hStdIn, &inputMode)) {
      SetConsoleMode(hStdIn, inputMode | ENABLE_VIRTUAL_TERMINAL_INPUT);
    }
  }

  platformInitialized = true;
  result.success = result.utf8Enabled; // Consider success if UTF-8 is enabled

#elif defined(__APPLE__)
  // macOS: Check for Homebrew paths and add if not present
  const char *path = getenv("PATH");
  if (path) {
    std::string pathStr(path);
    // Check for Apple Silicon Homebrew path
    if (pathStr.find("/opt/homebrew/bin") == std::string::npos) {
      struct stat st;
      if (stat("/opt/homebrew/bin", &st) == 0 && S_ISDIR(st.st_mode)) {
        std::string newPath = "/opt/homebrew/bin:" + pathStr;
        setenv("PATH", newPath.c_str(), 1);
      }
    }
    // Check for Intel Homebrew path
    if (pathStr.find("/usr/local/bin") == std::string::npos) {
      struct stat st;
      if (stat("/usr/local/bin", &st) == 0 && S_ISDIR(st.st_mode)) {
        const char *currentPath = getenv("PATH");
        std::string newPath = std::string(currentPath) + ":/usr/local/bin";
        setenv("PATH", newPath.c_str(), 1);
      }
    }
  }
  result.success = true;
  result.utf8Enabled = true;  // macOS terminals are UTF-8 by default
  result.vt100Enabled = true; // macOS terminals support VT100

#else
  // Linux: Terminals are typically UTF-8 and VT100 compatible
  result.success = true;
  result.utf8Enabled = true;
  result.vt100Enabled = true;
#endif

  return result;
}

void cleanupPlatform() {
#ifdef _WIN32
  if (platformInitialized) {
    // Restore original code pages
    if (originalInputCP != 0) {
      SetConsoleCP(originalInputCP);
    }
    if (originalOutputCP != 0) {
      SetConsoleOutputCP(originalOutputCP);
    }
    // Restore original output mode
    if (hStdOut != INVALID_HANDLE_VALUE && originalOutputMode != 0) {
      SetConsoleMode(hStdOut, originalOutputMode);
    }
    platformInitialized = false;
  }
#endif
  // No cleanup needed for macOS/Linux
}

} // namespace termidash
