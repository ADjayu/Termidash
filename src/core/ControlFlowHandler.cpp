#include "core/ControlFlowHandler.hpp"
#include "core/Parser.hpp"
#include "core/VariableManager.hpp"
#include "core/FunctionManager.hpp"
#include <iostream>

namespace termidash {

bool ControlFlowHandler::startsBlock(const std::string& cmd) {
    return cmd.rfind("if ", 0) == 0 ||
           cmd.rfind("while ", 0) == 0 ||
           cmd.rfind("for ", 0) == 0 ||
           cmd.rfind("function ", 0) == 0 ||
           cmd.find("()") != std::string::npos;
}

bool ControlFlowHandler::endsBlock(const std::string& cmd) {
    return cmd == "end" || cmd == "}";
}

bool ControlFlowHandler::isElse(const std::string& cmd) {
    return cmd == "else";
}

Block ControlFlowHandler::parseIf(const std::string& cmd) {
    Block b;
    b.type = BlockType::If;
    b.condition = cmd.substr(3); // Skip "if "
    return b;
}

Block ControlFlowHandler::parseWhile(const std::string& cmd) {
    Block b;
    b.type = BlockType::While;
    b.condition = cmd.substr(6); // Skip "while "
    return b;
}

Block ControlFlowHandler::parseFor(const std::string& cmd) {
    Block b;
    b.type = BlockType::For;
    
    // for var in item1 item2 ...
    std::string rest = cmd.substr(4); // Skip "for "
    size_t inPos = rest.find(" in ");
    if (inPos != std::string::npos) {
        b.loopVar = Parser::trim(rest.substr(0, inPos));
        std::string itemsStr = rest.substr(inPos + 4);
        
        // Split items
        std::string item;
        for (char c : itemsStr) {
            if (c == ' ') {
                if (!item.empty()) {
                    b.items.push_back(item);
                    item.clear();
                }
            } else {
                item += c;
            }
        }
        if (!item.empty()) {
            b.items.push_back(item);
        }
    }
    return b;
}

Block ControlFlowHandler::parseFunction(const std::string& cmd) {
    Block b;
    b.type = BlockType::Function;
    
    if (cmd.rfind("function ", 0) == 0) {
        std::string name = Parser::trim(cmd.substr(9));
        size_t bracePos = name.find('{');
        if (bracePos != std::string::npos) {
            name = Parser::trim(name.substr(0, bracePos));
        }
        b.condition = name; // Store function name in condition field
    } else {
        // name() { style
        size_t parenPos = cmd.find("()");
        if (parenPos != std::string::npos) {
            b.condition = Parser::trim(cmd.substr(0, parenPos));
        }
    }
    return b;
}

void ControlFlowHandler::registerFunction(const Block& block) {
    FunctionManager::instance().define(block.condition, block.body);
}

// Note: executeIf, executeWhile, executeFor are stubs.
// Actual execution is in ShellLoop.cpp which has access to processInputLine
// and PipelineExecutor. This handler provides parsing utilities only.

void ControlFlowHandler::executeIf(
    const Block& block,
    BuiltInCommandHandler& builtInHandler,
    ICommandExecutor* executor,
    platform::IProcessManager* processManager,
    IJobManager* jobManager,
    ShellState& state
) {
    // Stub: Actual execution is in ShellLoop
    (void)block;
    (void)builtInHandler;
    (void)executor;
    (void)processManager;
    (void)jobManager;
    (void)state;
}

void ControlFlowHandler::executeWhile(
    const Block& block,
    BuiltInCommandHandler& builtInHandler,
    ICommandExecutor* executor,
    platform::IProcessManager* processManager,
    IJobManager* jobManager,
    ShellState& state
) {
    // Stub: Actual execution is in ShellLoop
    (void)block;
    (void)builtInHandler;
    (void)executor;
    (void)processManager;
    (void)jobManager;
    (void)state;
}

void ControlFlowHandler::executeFor(
    const Block& block,
    BuiltInCommandHandler& builtInHandler,
    ICommandExecutor* executor,
    platform::IProcessManager* processManager,
    IJobManager* jobManager,
    ShellState& state
) {
    // Stub: Actual execution is in ShellLoop
    (void)block;
    (void)builtInHandler;
    (void)executor;
    (void)processManager;
    (void)jobManager;
    (void)state;
}

} // namespace termidash
