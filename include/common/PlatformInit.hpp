#pragma once

/**
 * @file PlatformInit.hpp
 * @brief Platform-specific initialization for Termidash
 *
 * This module handles platform-specific setup that must occur before
 * the shell starts, including:
 * - Windows: UTF-8 console encoding and VT100 escape sequence support
 * - macOS: Homebrew path detection
 * - All platforms: Environment normalization
 */

#include <string>

namespace termidash {

/**
 * @brief Platform initialization result
 */
struct PlatformInitResult {
  bool success = false;
  bool utf8Enabled = false;
  bool vt100Enabled = false;
  std::string errorMessage;
};

/**
 * @brief Initialize platform-specific settings
 *
 * Call this function once at program startup before creating
 * any terminal or process manager instances.
 *
 * Windows:
 * - Sets console input/output to UTF-8 (code page 65001)
 * - Enables Virtual Terminal Processing for ANSI color codes
 *
 * macOS:
 * - Adds /opt/homebrew/bin to PATH if it exists
 *
 * Linux:
 * - No special initialization required
 *
 * @return PlatformInitResult with success status and details
 */
PlatformInitResult initializePlatform();

/**
 * @brief Restore platform to original state
 *
 * Call this on program exit to restore original console settings.
 * This is optional but recommended for clean shutdown.
 */
void cleanupPlatform();

} // namespace termidash
