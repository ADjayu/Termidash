#pragma once
#include <string>
#include <functional>

namespace termidash {

/**
 * CommandSubstitution - Handles $(command) and `command` expansion
 * 
 * Supports nested substitutions: $(echo $(pwd))
 */
class CommandSubstitution {
public:
    using ExecuteFunc = std::function<std::string(const std::string&)>;
    
    /**
     * Expand all command substitutions in the input string.
     * @param input The input string containing $(cmd) or `cmd` patterns
     * @param executor Function that executes a command and returns its output
     * @return String with all substitutions expanded
     */
    static std::string expand(const std::string& input, ExecuteFunc executor);
    
    /**
     * Check if the input contains any command substitution patterns.
     */
    static bool hasSubstitution(const std::string& input);
    
private:
    /**
     * Find matching closing parenthesis for $(, handling nested parens.
     * @return Position of closing ) or std::string::npos if not found
     */
    static size_t findMatchingParen(const std::string& str, size_t openPos);
    
    /**
     * Find matching backtick, handling escape sequences.
     * @return Position of closing ` or std::string::npos if not found
     */
    static size_t findMatchingBacktick(const std::string& str, size_t openPos);
    
    /**
     * Convert backtick syntax to $() syntax for uniform handling.
     */
    static std::string convertBackticks(const std::string& input);
    
    /**
     * Expand a single $() substitution (recursive for nested).
     */
    static std::string expandDollarParen(const std::string& input, ExecuteFunc executor);
};

} // namespace termidash
