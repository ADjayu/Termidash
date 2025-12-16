#ifndef TERMIDASH_LOGGER_HPP
#define TERMIDASH_LOGGER_HPP

#include <string>

namespace termidash {

/**
 * @brief Logger wrapper class providing OS-standard log file locations
 *
 * Log locations:
 * - Windows: %APPDATA%\Termidash\logs\termidash.log
 * - macOS: ~/Library/Logs/Termidash/termidash.log
 * - Linux: ~/.local/share/termidash/logs/termidash.log
 */
class Logger {
public:
  enum class Level { Trace, Debug, Info, Warn, Error, Critical };

  /**
   * @brief Initialize the logging system
   * Creates log directory if it doesn't exist and sets up file + console sinks
   */
  static void init();

  /**
   * @brief Shutdown the logging system
   */
  static void shutdown();

  /**
   * @brief Log a trace message
   */
  static void trace(const std::string &msg);

  /**
   * @brief Log a debug message
   */
  static void debug(const std::string &msg);

  /**
   * @brief Log an info message
   */
  static void info(const std::string &msg);

  /**
   * @brief Log a warning message
   */
  static void warn(const std::string &msg);

  /**
   * @brief Log an error message
   */
  static void error(const std::string &msg);

  /**
   * @brief Log a critical message
   */
  static void critical(const std::string &msg);

  /**
   * @brief Get the OS-specific log directory path
   * @return Absolute path to log directory
   */
  static std::string getLogDirectory();

  /**
   * @brief Get the full path to the log file
   * @return Absolute path to log file
   */
  static std::string getLogFilePath();

  /**
   * @brief Set the minimum log level
   */
  static void setLevel(Level level);

  /**
   * @brief Check if logger has been initialized
   */
  static bool isInitialized();

private:
  static bool s_initialized;
};

} // namespace termidash

#endif // TERMIDASH_LOGGER_HPP
