#include "common/Logger.hpp"
#include <cstdlib>
#include <filesystem>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>


#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>

#else
#include <pwd.h>
#include <unistd.h>

#endif

namespace termidash {

bool Logger::s_initialized = false;

std::string Logger::getLogDirectory() {
  std::string logDir;

#ifdef _WIN32
  // Windows: %APPDATA%\Termidash\logs
  char *appData = nullptr;
  size_t len = 0;
  if (_dupenv_s(&appData, &len, "APPDATA") == 0 && appData != nullptr) {
    logDir = std::string(appData) + "\\Termidash\\logs";
    free(appData);
  } else {
    // Fallback to current directory
    logDir = ".\\logs";
  }
#elif defined(__APPLE__)
  // macOS: ~/Library/Logs/Termidash
  const char *home = getenv("HOME");
  if (home) {
    logDir = std::string(home) + "/Library/Logs/Termidash";
  } else {
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
      logDir = std::string(pw->pw_dir) + "/Library/Logs/Termidash";
    } else {
      logDir = "./logs";
    }
  }
#else
  // Linux: ~/.local/share/termidash/logs
  const char *xdgData = getenv("XDG_DATA_HOME");
  if (xdgData && strlen(xdgData) > 0) {
    logDir = std::string(xdgData) + "/termidash/logs";
  } else {
    const char *home = getenv("HOME");
    if (home) {
      logDir = std::string(home) + "/.local/share/termidash/logs";
    } else {
      struct passwd *pw = getpwuid(getuid());
      if (pw) {
        logDir = std::string(pw->pw_dir) + "/.local/share/termidash/logs";
      } else {
        logDir = "./logs";
      }
    }
  }
#endif

  return logDir;
}

std::string Logger::getLogFilePath() {
  return getLogDirectory() +
#ifdef _WIN32
         "\\termidash.log";
#else
         "/termidash.log";
#endif
}

void Logger::init() {
  if (s_initialized) {
    return;
  }

  try {
    // Create log directory if it doesn't exist
    std::string logDir = getLogDirectory();
    std::filesystem::create_directories(logDir);

    // Create sinks
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(
        spdlog::level::warn); // Console only shows warnings and above

    // Rotating file sink: 5MB max, 3 rotated files
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        getLogFilePath(), 5 * 1024 * 1024, 3);
    file_sink->set_level(spdlog::level::trace); // File captures everything

    // Create logger with both sinks
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("termidash", sinks.begin(),
                                                   sinks.end());

    logger->set_level(spdlog::level::trace);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");

    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::warn);

    s_initialized = true;

    info("Logger initialized. Log file: " + getLogFilePath());
  } catch (const spdlog::spdlog_ex &ex) {
    // If logging setup fails, we can't log it - just set flag
    s_initialized = false;
  }
}

void Logger::shutdown() {
  if (s_initialized) {
    spdlog::shutdown();
    s_initialized = false;
  }
}

void Logger::trace(const std::string &msg) {
  if (s_initialized) {
    spdlog::trace(msg);
  }
}

void Logger::debug(const std::string &msg) {
  if (s_initialized) {
    spdlog::debug(msg);
  }
}

void Logger::info(const std::string &msg) {
  if (s_initialized) {
    spdlog::info(msg);
  }
}

void Logger::warn(const std::string &msg) {
  if (s_initialized) {
    spdlog::warn(msg);
  }
}

void Logger::error(const std::string &msg) {
  if (s_initialized) {
    spdlog::error(msg);
  }
}

void Logger::critical(const std::string &msg) {
  if (s_initialized) {
    spdlog::critical(msg);
  }
}

void Logger::setLevel(Level level) {
  if (!s_initialized)
    return;

  spdlog::level::level_enum spdlogLevel;
  switch (level) {
  case Level::Trace:
    spdlogLevel = spdlog::level::trace;
    break;
  case Level::Debug:
    spdlogLevel = spdlog::level::debug;
    break;
  case Level::Info:
    spdlogLevel = spdlog::level::info;
    break;
  case Level::Warn:
    spdlogLevel = spdlog::level::warn;
    break;
  case Level::Error:
    spdlogLevel = spdlog::level::err;
    break;
  case Level::Critical:
    spdlogLevel = spdlog::level::critical;
    break;
  default:
    spdlogLevel = spdlog::level::info;
    break;
  }
  spdlog::set_level(spdlogLevel);
}

bool Logger::isInitialized() { return s_initialized; }

} // namespace termidash
