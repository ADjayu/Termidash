#include "core/CommandSubstitution.hpp"
#include <stdexcept>
#include <algorithm>

namespace termidash {

bool CommandSubstitution::hasSubstitution(const std::string& input) {
    // Check for $( pattern, but NOT $(( which is arithmetic expansion
    for (size_t i = 0; i < input.size(); ++i) {
        if (i + 1 < input.size() && input[i] == '$' && input[i + 1] == '(') {
            // Check if this is $(( arithmetic expansion
            if (i + 2 < input.size() && input[i + 2] == '(') {
                // This is $(( - skip past it
                continue;
            }
            // Found $( that's not $((
            return true;
        }
    }
    // Check for backticks (not escaped)
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '`') {
            // Check if escaped
            size_t backslashes = 0;
            size_t j = i;
            while (j > 0 && input[j - 1] == '\\') {
                --j;
                ++backslashes;
            }
            if (backslashes % 2 == 0) {
                return true;
            }
        }
    }
    return false;
}

size_t CommandSubstitution::findMatchingParen(const std::string& str, size_t openPos) {
    int depth = 1;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    
    for (size_t i = openPos + 1; i < str.size(); ++i) {
        char c = str[i];
        char prev = (i > 0) ? str[i - 1] : '\0';
        
        // Handle escape sequences
        if (prev == '\\') {
            continue;
        }
        
        // Handle quotes
        if (c == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
            continue;
        }
        if (c == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }
        
        // Skip if inside quotes
        if (inSingleQuote || inDoubleQuote) {
            continue;
        }
        
        // Handle nested $()
        if (c == '(' && i > 0 && str[i - 1] == '$') {
            ++depth;
        } else if (c == ')') {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    
    return std::string::npos;
}

size_t CommandSubstitution::findMatchingBacktick(const std::string& str, size_t openPos) {
    for (size_t i = openPos + 1; i < str.size(); ++i) {
        if (str[i] == '`') {
            // Check if escaped
            size_t backslashes = 0;
            size_t j = i;
            while (j > 0 && str[j - 1] == '\\') {
                --j;
                ++backslashes;
            }
            if (backslashes % 2 == 0) {
                return i;
            }
        }
    }
    return std::string::npos;
}

std::string CommandSubstitution::convertBackticks(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    
    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == '`') {
            // Check if escaped
            size_t backslashes = 0;
            size_t j = i;
            while (j > 0 && result.size() > 0 && result.back() == '\\') {
                result.pop_back();
                ++backslashes;
            }
            
            if (backslashes % 2 == 0) {
                // Find matching backtick
                size_t closePos = findMatchingBacktick(input, i);
                if (closePos != std::string::npos) {
                    // Extract command and convert to $()
                    std::string cmd = input.substr(i + 1, closePos - i - 1);
                    result += "$(" + cmd + ")";
                    i = closePos + 1;
                    continue;
                }
            }
        }
        result += input[i];
        ++i;
    }
    
    return result;
}

std::string CommandSubstitution::expandDollarParen(const std::string& input, ExecuteFunc executor) {
    std::string result;
    result.reserve(input.size());
    
    size_t i = 0;
    while (i < input.size()) {
        // Check for $( pattern
        if (i + 1 < input.size() && input[i] == '$' && input[i + 1] == '(') {
            // Find matching closing paren (handles nested)
            size_t closePos = findMatchingParen(input, i + 1);
            
            if (closePos == std::string::npos) {
                // No matching paren, keep as-is
                result += input[i];
                ++i;
                continue;
            }
            
            // Extract the command (between $( and ))
            std::string cmd = input.substr(i + 2, closePos - i - 2);
            
            // Recursively expand any nested substitutions first
            if (hasSubstitution(cmd)) {
                cmd = expand(cmd, executor);
            }
            
            // Execute the command and get output
            std::string output = executor(cmd);
            
            // Remove trailing newlines from output
            while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
                output.pop_back();
            }
            
            result += output;
            i = closePos + 1;
        } else {
            result += input[i];
            ++i;
        }
    }
    
    return result;
}

std::string CommandSubstitution::expand(const std::string& input, ExecuteFunc executor) {
    if (!hasSubstitution(input)) {
        return input;
    }
    
    // First, convert all backticks to $() syntax
    std::string converted = convertBackticks(input);
    
    // Then expand all $() substitutions (handles nested via recursion)
    return expandDollarParen(converted, executor);
}

} // namespace termidash
