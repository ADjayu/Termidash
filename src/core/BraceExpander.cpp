#include "core/BraceExpander.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace termidash {

bool BraceExpander::hasBraces(const std::string& input) {
    // Look for { followed by } with content in between
    size_t openPos = input.find('{');
    if (openPos == std::string::npos) {
        return false;
    }
    size_t closePos = findMatchingBrace(input, openPos);
    return closePos != std::string::npos;
}

size_t BraceExpander::findMatchingBrace(const std::string& str, size_t openPos) {
    int depth = 1;
    bool escaped = false;
    
    for (size_t i = openPos + 1; i < str.size(); ++i) {
        if (escaped) {
            escaped = false;
            continue;
        }
        if (str[i] == '\\') {
            escaped = true;
            continue;
        }
        if (str[i] == '{') {
            ++depth;
        } else if (str[i] == '}') {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    
    return std::string::npos;
}

bool BraceExpander::isRange(const std::string& content) {
    // Check for patterns like "1..5" or "a..z"
    size_t dotDot = content.find("..");
    if (dotDot == std::string::npos || dotDot == 0 || dotDot == content.size() - 2) {
        return false;
    }
    
    std::string start = content.substr(0, dotDot);
    std::string end = content.substr(dotDot + 2);
    
    // Trim whitespace
    while (!start.empty() && std::isspace(static_cast<unsigned char>(start.back()))) start.pop_back();
    while (!end.empty() && std::isspace(static_cast<unsigned char>(end.front()))) end.erase(0, 1);
    
    if (start.empty() || end.empty()) {
        return false;
    }
    
    // Both numeric or both single character
    bool startIsNum = true, endIsNum = true;
    for (char c : start) if (!std::isdigit(static_cast<unsigned char>(c)) && c != '-') startIsNum = false;
    for (char c : end) if (!std::isdigit(static_cast<unsigned char>(c)) && c != '-') endIsNum = false;
    
    if (startIsNum && endIsNum) {
        return true;
    }
    
    // Single character range
    if (start.size() == 1 && end.size() == 1 && std::isalpha(static_cast<unsigned char>(start[0])) && 
        std::isalpha(static_cast<unsigned char>(end[0]))) {
        return true;
    }
    
    return false;
}

std::vector<std::string> BraceExpander::expandRange(const std::string& rangeSpec) {
    std::vector<std::string> result;
    
    size_t dotDot = rangeSpec.find("..");
    if (dotDot == std::string::npos) {
        return result;
    }
    
    std::string startStr = rangeSpec.substr(0, dotDot);
    std::string endStr = rangeSpec.substr(dotDot + 2);
    
    // Trim whitespace
    while (!startStr.empty() && std::isspace(static_cast<unsigned char>(startStr.back()))) startStr.pop_back();
    while (!endStr.empty() && std::isspace(static_cast<unsigned char>(endStr.front()))) endStr.erase(0, 1);
    
    // Check for numeric range
    bool isNumeric = true;
    for (char c : startStr) if (!std::isdigit(static_cast<unsigned char>(c)) && c != '-') isNumeric = false;
    for (char c : endStr) if (!std::isdigit(static_cast<unsigned char>(c)) && c != '-') isNumeric = false;
    
    if (isNumeric && !startStr.empty() && !endStr.empty()) {
        try {
            int start = std::stoi(startStr);
            int end = std::stoi(endStr);
            int step = (start <= end) ? 1 : -1;
            
            for (int i = start; step > 0 ? i <= end : i >= end; i += step) {
                result.push_back(std::to_string(i));
            }
        } catch (...) {
            // Invalid number, return empty
        }
    } 
    // Character range
    else if (startStr.size() == 1 && endStr.size() == 1) {
        char start = startStr[0];
        char end = endStr[0];
        int step = (start <= end) ? 1 : -1;
        
        for (char c = start; step > 0 ? c <= end : c >= end; c += step) {
            result.push_back(std::string(1, c));
        }
    }
    
    return result;
}

std::vector<std::string> BraceExpander::splitByComma(const std::string& content) {
    std::vector<std::string> result;
    std::string current;
    int braceDepth = 0;
    bool escaped = false;
    
    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        
        if (escaped) {
            current += c;
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            escaped = true;
            current += c;
            continue;
        }
        
        if (c == '{') {
            ++braceDepth;
            current += c;
        } else if (c == '}') {
            --braceDepth;
            current += c;
        } else if (c == ',' && braceDepth == 0) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

std::vector<std::string> BraceExpander::expandBraceContent(
    const std::string& prefix,
    const std::string& braceContent,
    const std::string& suffix) {
    
    std::vector<std::string> result;
    
    // Check if this is a range
    if (isRange(braceContent)) {
        std::vector<std::string> rangeItems = expandRange(braceContent);
        for (const auto& item : rangeItems) {
            // Recursively expand suffix (may contain more braces)
            std::string combined = prefix + item + suffix;
            std::vector<std::string> expanded = expand(combined);
            result.insert(result.end(), expanded.begin(), expanded.end());
        }
    } else {
        // Comma-separated list
        std::vector<std::string> items = splitByComma(braceContent);
        
        // If no comma found, treat as literal (no expansion needed)
        if (items.size() <= 1) {
            std::string combined = prefix + braceContent + suffix;
            // Still expand in case braceContent has nested braces
            std::vector<std::string> expanded = expand(combined);
            result.insert(result.end(), expanded.begin(), expanded.end());
        } else {
            for (const auto& item : items) {
                // Recursively expand each item (may have nested braces)
                std::string combined = prefix + item + suffix;
                std::vector<std::string> expanded = expand(combined);
                result.insert(result.end(), expanded.begin(), expanded.end());
            }
        }
    }
    
    return result;
}

std::vector<std::string> BraceExpander::expand(const std::string& input) {
    std::vector<std::string> result;
    
    // Find the first { that has a matching }
    size_t openPos = std::string::npos;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '{') {
            size_t closePos = findMatchingBrace(input, i);
            if (closePos != std::string::npos) {
                openPos = i;
                break;
            }
        }
    }
    
    if (openPos == std::string::npos) {
        // No braces to expand
        result.push_back(input);
        return result;
    }
    
    size_t closePos = findMatchingBrace(input, openPos);
    
    std::string prefix = input.substr(0, openPos);
    std::string braceContent = input.substr(openPos + 1, closePos - openPos - 1);
    std::string suffix = input.substr(closePos + 1);
    
    return expandBraceContent(prefix, braceContent, suffix);
}

} // namespace termidash
