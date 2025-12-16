#ifndef TERMIDASH_SECURITY_UTILS_HPP
#define TERMIDASH_SECURITY_UTILS_HPP

#include <string>
#include <vector>

namespace termidash {
namespace security {

/**
 * @brief Sanitize user input by removing control characters
 * @param input Raw user input
 * @return Sanitized string safe for processing
 */
std::string sanitizeInput(const std::string &input);

/**
 * @brief Check if a path is safe (no directory traversal attacks)
 * @param path Path to validate
 * @return true if path is safe, false if it contains traversal patterns
 */
bool isPathSafe(const std::string &path);

/**
 * @brief Mask sensitive arguments in a command for history storage
 * Detects patterns like password=xxx, token=xxx, secret=xxx
 * @param command Command line to mask
 * @return Command with sensitive values replaced with ***
 */
std::string maskSensitiveArgs(const std::string &command);

/**
 * @brief Check if safe mode is enabled
 * In safe mode, dangerous operations are blocked
 * @return true if safe mode is enabled
 */
bool isSafeModeEnabled();

/**
 * @brief Enable or disable safe mode
 * @param enabled true to enable safe mode
 */
void setSafeMode(bool enabled);

/**
 * @brief Check if a command is allowed in safe mode
 * @param command Command to check
 * @return true if command is allowed, false if blocked
 */
bool isCommandAllowedInSafeMode(const std::string &command);

/**
 * @brief Get list of commands blocked in safe mode
 * @return Vector of blocked command names
 */
std::vector<std::string> getBlockedCommands();

} // namespace security
} // namespace termidash

#endif // TERMIDASH_SECURITY_UTILS_HPP
