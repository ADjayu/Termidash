#pragma once

#include "core/CommandExecutor.hpp"

class WindowsCommandExecutor : public ICommandExecutor {
public:
    int execute(const std::string& command, bool background) override;
    std::string getLastError() override;

private:
    std::string lastError;
};
