#include "common/Logger.hpp"
#include "common/PlatformInit.hpp"
#include "common/SecurityUtils.hpp"
#include "core/PlatformFactory.hpp"
#include "core/ShellLoop.hpp"
#include <cstring>
#include <iostream>


void printUsage(const char *programName) {
  std::cout
      << "Usage: " << programName << " [options] [script_file]\n"
      << "\nOptions:\n"
      << "  -c <command>    Execute a single command and exit\n"
      << "  --safe-mode     Run in safe mode (blocks dangerous commands)\n"
      << "  --help, -h      Show this help message\n"
      << "  --version, -v   Show version information\n";
}

void printVersion() {
  std::cout << "Termidash Shell Version 1.0.0\n"
            << "A modern, cross-platform command-line shell\n";
}

int main(int argc, char *argv[]) {
  // Initialize logger first
  termidash::Logger::init();
  termidash::Logger::info("Termidash starting...");

  // Parse command line arguments
  bool safeMode = false;
  std::string command;
  std::string scriptPath;
  bool runCommand = false;
  bool runScript = false;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--safe-mode") == 0) {
      safeMode = true;
      termidash::Logger::info("Safe mode enabled");
    } else if (std::strcmp(argv[i], "--help") == 0 ||
               std::strcmp(argv[i], "-h") == 0) {
      printUsage(argv[0]);
      termidash::Logger::shutdown();
      return 0;
    } else if (std::strcmp(argv[i], "--version") == 0 ||
               std::strcmp(argv[i], "-v") == 0) {
      printVersion();
      termidash::Logger::shutdown();
      return 0;
    } else if (std::strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      command = argv[++i];
      runCommand = true;
    } else if (argv[i][0] != '-') {
      scriptPath = argv[i];
      runScript = true;
    }
  }

  // Enable safe mode if requested
  if (safeMode) {
    termidash::security::setSafeMode(true);
    std::cout << "Safe mode enabled. Dangerous commands are blocked.\n";
  }

  // Initialize platform-specific settings (UTF-8, VT100, etc.)
  auto initResult = termidash::initializePlatform();
  if (!initResult.success) {
    termidash::Logger::warn("Platform initialization incomplete: " +
                            initResult.errorMessage);
    std::cerr << "Warning: Platform initialization incomplete: "
              << initResult.errorMessage << std::endl;
  }

  auto terminal = termidash::createTerminal();
  auto processManager = termidash::createProcessManager();

  if (!terminal || !processManager) {
    termidash::Logger::error("Failed to initialize platform components");
    std::cerr << "Failed to initialize platform components!" << std::endl;
    termidash::cleanupPlatform();
    termidash::Logger::shutdown();
    return 1;
  }

  int exitCode = 0;

  if (runCommand) {
    termidash::Logger::info("Executing command: " +
                            termidash::security::maskSensitiveArgs(command));
    termidash::runCommand(command, terminal.get(), processManager.get());
  } else if (runScript) {
    termidash::Logger::info("Executing script: " + scriptPath);
    termidash::runScript(scriptPath, terminal.get(), processManager.get());
  } else {
    termidash::Logger::info("Starting interactive shell");
    termidash::runShell(terminal.get(), processManager.get());
  }

  termidash::Logger::info("Termidash shutting down");
  termidash::cleanupPlatform();
  termidash::Logger::shutdown();
  return exitCode;
}
