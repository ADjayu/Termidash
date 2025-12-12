#include "common/PlatformInit.hpp"
#include "core/PlatformFactory.hpp"
#include "core/ShellLoop.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  // Initialize platform-specific settings (UTF-8, VT100, etc.)
  auto initResult = termidash::initializePlatform();
  if (!initResult.success) {
    std::cerr << "Warning: Platform initialization incomplete: "
              << initResult.errorMessage << std::endl;
  }

  auto terminal = termidash::createTerminal();
  auto processManager = termidash::createProcessManager();

  if (!terminal || !processManager) {
    std::cerr << "Failed to initialize platform components!" << std::endl;
    termidash::cleanupPlatform();
    return 1;
  }

  if (argc > 2 && std::string(argv[1]) == "-c") {
    std::string command = argv[2];
    termidash::runCommand(command, terminal.get(), processManager.get());
  } else if (argc == 2) {
    std::string scriptPath = argv[1];
    termidash::runScript(scriptPath, terminal.get(), processManager.get());
  } else {
    termidash::runShell(terminal.get(), processManager.get());
  }

  termidash::cleanupPlatform();
  return 0;
}
