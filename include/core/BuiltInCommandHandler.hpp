#pragma once
#include <string>
#include <vector>
#include "core/BuiltIn/CommonCommandHandler.hpp"
#ifdef PLATFORM_WINDOWS
#include "core/BuiltIn/WindowsCommandHandler.hpp"
#endif
#include "core/ExecContext.hpp"
#include "core/Environment.hpp"

namespace termidash {

class BuiltInCommandHandler {
public:
    BuiltInCommandHandler();
    bool handleCommand(const std::string& input);
    int handleCommandWithContext(const std::string& input, ExecContext& ctx);
    const std::vector<std::string>& getHistory() const;
    std::vector<std::string> tokenize(const std::string& input) const;
    bool isBuiltInCommand(const std::string &input) const;

    CommonCommandHandler commonHandler;
#ifdef PLATFORM_WINDOWS
    WindowsCommandHandler windowsHandler;
#endif
};

} // namespace termidash
