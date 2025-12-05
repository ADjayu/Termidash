#pragma once
#include <string>
#include <vector>
#include "core/ExecContext.hpp"

namespace termidash {

class WindowsCommandHandler {
public:
    bool isCommand(const std::string& cmd) const;
    WindowsCommandHandler() = default;
    bool handle(const std::vector<std::string>& tokens);
    int handleWithContext(const std::vector<std::string>& tokens, ExecContext& ctx);
};

} // namespace termidash
