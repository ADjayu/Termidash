#include <iostream>
#include "core/ShellLoop.hpp"
#include "core/PlatformFactory.hpp"

int main(int argc, char* argv[])
{
    auto terminal = termidash::createTerminal();
    auto processManager = termidash::createProcessManager();

    if (!terminal || !processManager) {
        std::cerr << "Failed to initialize platform components!" << std::endl;
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

    return 0;
}
