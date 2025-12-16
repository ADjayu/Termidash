#include "common/SecurityUtils.hpp"
#include <algorithm>
#include <cctype>
#include <regex>


namespace termidash {
namespace security {

// Global safe mode flag
static bool g_safeModeEnabled = false;

// List of commands blocked in safe mode
static const std::vector<std::string> BLOCKED_COMMANDS = {
    "rm",       "del",      "rmdir", "rd", // File/directory deletion
    "format",                              // Disk formatting
    "mkfs",                                // Linux filesystem creation
    "dd",                                  // Raw disk operations
    "chmod",    "chown",                   // Permission changes
    "kill",     "taskkill",                // Process termination
    "shutdown", "reboot",   "halt",        // System control
    "curl",     "wget",                    // Network downloads
    "ssh",      "scp",      "sftp",        // Remote connections
    "sudo",     "su",       "runas"        // Privilege escalation
};

// Sensitive argument patterns to mask in history
static const std::vector<std::string> SENSITIVE_PATTERNS = {
    "password", "passwd",     "pwd",  "token", "api_key", "apikey",
    "secret",   "credential", "cred", "auth",  "key",     "private"};

std::string sanitizeInput(const std::string &input) {
  std::string result;
  result.reserve(input.size());

  for (char c : input) {
    // Allow printable ASCII and common whitespace
    if (std::isprint(static_cast<unsigned char>(c)) || c == '\n' || c == '\r' ||
        c == '\t') {
      result += c;
    }
    // Skip control characters (0x00-0x1F except tab/newline, and 0x7F)
  }

  return result;
}

bool isPathSafe(const std::string &path) {
  // Check for directory traversal patterns
  if (path.find("..") != std::string::npos) {
    return false;
  }

  // Check for absolute paths trying to access system directories
  // Windows system paths
  if (path.find("C:\\Windows") != std::string::npos ||
      path.find("C:/Windows") != std::string::npos ||
      path.find("\\Windows\\System32") != std::string::npos) {
    return false;
  }

  // Unix system paths
  if (path.find("/etc/") != std::string::npos ||
      path.find("/usr/") != std::string::npos ||
      path.find("/bin/") != std::string::npos ||
      path.find("/sbin/") != std::string::npos ||
      path.find("/root/") != std::string::npos) {
    return false;
  }

  return true;
}

std::string maskSensitiveArgs(const std::string &command) {
  std::string result = command;

  for (const auto &pattern : SENSITIVE_PATTERNS) {
    // Match patterns like: pattern=value, pattern="value", pattern='value'
    // Case-insensitive matching
    std::string regexPattern =
        "(" + pattern + ")([=:])([\"']?)([^\"'\\s]+)([\"']?)";

    try {
      std::regex re(regexPattern, std::regex::icase);
      result = std::regex_replace(result, re, "$1$2$3***$5");
    } catch (const std::regex_error &) {
      // If regex fails, continue with other patterns
      continue;
    }
  }

  return result;
}

bool isSafeModeEnabled() { return g_safeModeEnabled; }

void setSafeMode(bool enabled) { g_safeModeEnabled = enabled; }

bool isCommandAllowedInSafeMode(const std::string &command) {
  if (!g_safeModeEnabled) {
    return true; // All commands allowed when not in safe mode
  }

  // Extract the command name (first word)
  std::string cmdName;
  size_t spacePos = command.find(' ');
  if (spacePos != std::string::npos) {
    cmdName = command.substr(0, spacePos);
  } else {
    cmdName = command;
  }

  // Convert to lowercase for comparison
  std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  // Check if command is in blocked list
  for (const auto &blocked : BLOCKED_COMMANDS) {
    if (cmdName == blocked) {
      return false;
    }
  }

  return true;
}

std::vector<std::string> getBlockedCommands() { return BLOCKED_COMMANDS; }

} // namespace security
} // namespace termidash
