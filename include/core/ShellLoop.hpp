#pragma once
#include "platform/interfaces/ITerminal.hpp"
#include "platform/interfaces/IProcessManager.hpp"

namespace termidash {
    void runShell(platform::ITerminal* terminal, platform::IProcessManager* processManager);
    void runCommand(const std::string& commandLine, platform::ITerminal* terminal, platform::IProcessManager* processManager);
    void runScript(const std::string& path, platform::ITerminal* terminal, platform::IProcessManager* processManager);
}
