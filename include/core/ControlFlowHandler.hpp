#pragma once
#include "core/Parser.hpp"
#include "core/BuiltInCommandHandler.hpp"
#include "core/CommandExecutor.hpp"
#include "core/JobManager.hpp"
#include "platform/interfaces/IProcessManager.hpp"
#include "platform/interfaces/ITerminal.hpp"
#include <string>
#include <vector>
#include <istream>

namespace termidash {

/**
 * @brief Control flow block types
 */
enum class BlockType {
    If,
    While,
    For,
    Function
};

/**
 * @brief Represents a control flow block (if, while, for, function)
 */
struct Block {
    BlockType type;
    std::string condition;      // For If/While - the condition command
    std::string loopVar;        // For For loop - the iteration variable
    std::vector<std::string> items;     // For For loop - the items to iterate
    std::vector<std::string> body;      // Commands in the block body
    std::vector<std::string> elseBody;  // Commands in else branch (for If)
    bool inElse = false;                // Currently in else branch
};

/**
 * @brief Shell state tracking nested blocks
 */
struct ShellState {
    std::vector<Block> blockStack;
    
    bool inBlock() const { return !blockStack.empty(); }
    
    Block& currentBlock() { return blockStack.back(); }
    const Block& currentBlock() const { return blockStack.back(); }
};

/**
 * @brief Control flow handler for shell blocks
 * 
 * Handles parsing and execution of:
 * - if/else/end statements
 * - while/end loops
 * - for/end loops
 * - function definitions
 */
class ControlFlowHandler {
public:
    /**
     * @brief Check if input starts a new block
     */
    static bool startsBlock(const std::string& cmd);

    /**
     * @brief Check if input ends a block
     */
    static bool endsBlock(const std::string& cmd);

    /**
     * @brief Check if input is 'else' keyword
     */
    static bool isElse(const std::string& cmd);

    /**
     * @brief Parse an if statement and create block
     * @param cmd Command starting with "if "
     * @return Configured If block
     */
    static Block parseIf(const std::string& cmd);

    /**
     * @brief Parse a while statement and create block
     * @param cmd Command starting with "while "
     * @return Configured While block
     */
    static Block parseWhile(const std::string& cmd);

    /**
     * @brief Parse a for statement and create block
     * @param cmd Command starting with "for "
     * @return Configured For block
     */
    static Block parseFor(const std::string& cmd);

    /**
     * @brief Parse a function definition and create block
     * @param cmd Command starting with "function " or containing "()"
     * @return Configured Function block
     */
    static Block parseFunction(const std::string& cmd);

    /**
     * @brief Execute an If block
     */
    static void executeIf(
        const Block& block,
        BuiltInCommandHandler& builtInHandler,
        ICommandExecutor* executor,
        platform::IProcessManager* processManager,
        IJobManager* jobManager,
        ShellState& state
    );

    /**
     * @brief Execute a While block
     */
    static void executeWhile(
        const Block& block,
        BuiltInCommandHandler& builtInHandler,
        ICommandExecutor* executor,
        platform::IProcessManager* processManager,
        IJobManager* jobManager,
        ShellState& state
    );

    /**
     * @brief Execute a For block
     */
    static void executeFor(
        const Block& block,
        BuiltInCommandHandler& builtInHandler,
        ICommandExecutor* executor,
        platform::IProcessManager* processManager,
        IJobManager* jobManager,
        ShellState& state
    );

    /**
     * @brief Register a function definition
     */
    static void registerFunction(const Block& block);
};

} // namespace termidash
