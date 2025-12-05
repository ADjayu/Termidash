#pragma once

#include <string>

class ICommandExecutor {
public:
    virtual int execute(const std::string& command, bool background) = 0;
    virtual std::string getLastError() = 0;
    virtual ~ICommandExecutor() {}
};
