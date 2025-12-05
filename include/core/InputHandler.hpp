#pragma once
#include "core/Parser.hpp"
#include "core/CompletionEngine.hpp"
#include "platform/interfaces/ITerminal.hpp"
#include <string>
#include <vector>
#include <functional>

namespace termidash {

/**
 * @brief Interactive input handler for shell line editing
 * 
 * Handles raw input, history navigation, and tab completion.
 */
class InputHandler {
public:
    /**
     * @brief Read a line with history navigation and tab completion
     * 
     * Supports:
     * - Arrow keys for history navigation
     * - Backspace for editing
     * - Tab for completion
     * 
     * @param terminal Terminal interface for I/O
     * @param history Command history
     * @param historyIndex Current position in history (updated)
     * @param completionGenerator Function to generate completions
     * @return The input line
     */
    static std::string readLine(
        platform::ITerminal* terminal,
        const std::vector<std::string>& history,
        size_t& historyIndex,
        std::function<std::vector<std::string>(const std::string&)> completionGenerator
    );
};

} // namespace termidash
