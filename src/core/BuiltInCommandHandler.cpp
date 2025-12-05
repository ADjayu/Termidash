#include "core/BuiltInCommandHandler.hpp"

namespace termidash {

BuiltInCommandHandler::BuiltInCommandHandler() {}

bool BuiltInCommandHandler::handleCommand(const std::string& input) {
    ExecContext ctx(std::cin, std::cout, std::cerr);
    return handleCommandWithContext(input, ctx);
}

int BuiltInCommandHandler::handleCommandWithContext(const std::string& input, ExecContext& ctx) {
    auto tokens = tokenize(input);
    if (tokens.empty()) return -1;

    int ret = commonHandler.handleWithContext(input, tokens, ctx);
    if (ret != -1) return ret;

#ifdef PLATFORM_WINDOWS
    ret = windowsHandler.handleWithContext(tokens, ctx);
    if (ret != -1) return ret;
#endif
    return -1;
}

const std::vector<std::string>& BuiltInCommandHandler::getHistory() const {
    return commonHandler.getHistory();
}

std::vector<std::string> BuiltInCommandHandler::tokenize(const std::string& input) const {
    return commonHandler.tokenize(input);
}



bool BuiltInCommandHandler::isBuiltInCommand(const std::string &input) const {
    auto toks = tokenize(input);
    if (toks.empty()) return false;
    const std::string &cmd = toks[0];
    if (commonHandler.isCommand(cmd)) return true;
#ifdef PLATFORM_WINDOWS
    if (windowsHandler.isCommand(cmd)) return true;
#endif
    return false;
}

} // namespace termidash
