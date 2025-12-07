#include "core/GlobExpander.hpp"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace termidash {

bool GlobExpander::hasGlobChars(const std::string& input) {
    for (char c : input) {
        if (c == '*' || c == '?' || c == '[') {
            return true;
        }
    }
    return false;
}

size_t GlobExpander::matchCharClass(const std::string& pattern, size_t pos, char c) {
    // Expects pattern[pos] == '['
    if (pos >= pattern.size() || pattern[pos] != '[') {
        return 0;
    }
    
    bool negate = false;
    size_t i = pos + 1;
    
    if (i < pattern.size() && (pattern[i] == '!' || pattern[i] == '^')) {
        negate = true;
        ++i;
    }
    
    bool matched = false;
    bool firstChar = true;
    char prevChar = 0;
    bool inRange = false;
    
    while (i < pattern.size()) {
        char pc = pattern[i];
        
        // ] ends the class, but not if it's the first char (allows []] to match ])
        if (pc == ']' && !firstChar) {
            if (negate) {
                return matched ? 0 : (i - pos + 1);
            } else {
                return matched ? (i - pos + 1) : 0;
            }
        }
        
        // Handle range like a-z
        if (pc == '-' && !firstChar && i + 1 < pattern.size() && pattern[i + 1] != ']') {
            inRange = true;
            ++i;
            continue;
        }
        
        if (inRange) {
            // prevChar - pc is the range
            if (c >= prevChar && c <= pc) {
                matched = true;
            }
            inRange = false;
        } else {
            if (c == pc) {
                matched = true;
            }
            prevChar = pc;
        }
        
        firstChar = false;
        ++i;
    }
    
    // Unclosed bracket - no match
    return 0;
}

bool GlobExpander::matchPattern(const std::string& pattern, const std::string& str) {
    size_t pi = 0, si = 0;
    size_t starPi = std::string::npos, starSi = 0;
    
    while (si < str.size()) {
        if (pi < pattern.size() && pattern[pi] == '*') {
            // Handle ** (match anything including paths)
            if (pi + 1 < pattern.size() && pattern[pi + 1] == '*') {
                // ** matches anything
                ++pi;
            }
            // Remember the star position for backtracking
            starPi = pi;
            starSi = si;
            ++pi;
        } else if (pi < pattern.size() && pattern[pi] == '?') {
            // ? matches any single character (but not path separator)
            if (str[si] != '/' && str[si] != '\\') {
                ++pi;
                ++si;
            } else if (starPi != std::string::npos) {
                pi = starPi + 1;
                si = ++starSi;
            } else {
                return false;
            }
        } else if (pi < pattern.size() && pattern[pi] == '[') {
            // Character class
            size_t consumed = matchCharClass(pattern, pi, str[si]);
            if (consumed > 0) {
                pi += consumed;
                ++si;
            } else if (starPi != std::string::npos) {
                pi = starPi + 1;
                si = ++starSi;
            } else {
                return false;
            }
        } else if (pi < pattern.size() && (pattern[pi] == str[si] || 
                   (pattern[pi] == '/' && str[si] == '\\') ||
                   (pattern[pi] == '\\' && str[si] == '/'))) {
            // Exact match (treat / and \ as equivalent)
            ++pi;
            ++si;
        } else if (starPi != std::string::npos) {
            // Backtrack to last * and try matching more characters
            pi = starPi + 1;
            si = ++starSi;
        } else {
            return false;
        }
    }
    
    // Consume remaining stars in pattern
    while (pi < pattern.size() && pattern[pi] == '*') {
        ++pi;
    }
    
    return pi == pattern.size();
}

void GlobExpander::splitPath(const std::string& path, 
    std::string& directory, std::string& filename) {
    
    size_t lastSep = path.find_last_of("/\\");
    if (lastSep == std::string::npos) {
        directory = ".";
        filename = path;
    } else {
        directory = path.substr(0, lastSep);
        if (directory.empty()) {
            directory = "/";
        }
        filename = path.substr(lastSep + 1);
    }
}

std::vector<std::string> GlobExpander::expandInDirectory(
    const std::string& directory,
    const std::string& pattern) {
    
    std::vector<std::string> result;
    
    try {
        fs::path dirPath(directory);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
            return result;
        }
        
        bool matchDotFiles = !pattern.empty() && pattern[0] == '.';
        
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            std::string filename = entry.path().filename().string();
            
            // Skip hidden files unless pattern starts with .
            if (!matchDotFiles && !filename.empty() && filename[0] == '.') {
                continue;
            }
            
            if (matchPattern(pattern, filename)) {
                if (directory == ".") {
                    result.push_back(filename);
                } else {
                    result.push_back(directory + "/" + filename);
                }
            }
        }
    } catch (const fs::filesystem_error&) {
        // Permission denied or other error - return empty
    }
    
    // Sort results for consistent output
    std::sort(result.begin(), result.end());
    
    return result;
}

std::vector<std::string> GlobExpander::expandRecursive(
    const std::string& basePath,
    const std::vector<std::string>& patternParts,
    size_t partIndex) {
    
    std::vector<std::string> result;
    
    if (partIndex >= patternParts.size()) {
        // We've matched all parts
        if (basePath.empty()) {
            return result;
        }
        try {
            if (fs::exists(basePath)) {
                result.push_back(basePath);
            }
        } catch (...) {}
        return result;
    }
    
    const std::string& part = patternParts[partIndex];
    
    if (part == "**") {
        // ** matches any depth of directories
        try {
            fs::path base(basePath.empty() ? "." : basePath);
            if (fs::exists(base) && fs::is_directory(base)) {
                // Match current level (skip **)
                auto subResult = expandRecursive(basePath, patternParts, partIndex + 1);
                result.insert(result.end(), subResult.begin(), subResult.end());
                
                // Recurse into subdirectories
                for (const auto& entry : fs::directory_iterator(base)) {
                    if (entry.is_directory()) {
                        std::string subPath = basePath.empty() ? 
                            entry.path().filename().string() :
                            basePath + "/" + entry.path().filename().string();
                        
                        // Continue with ** at same position (matches more levels)
                        auto r1 = expandRecursive(subPath, patternParts, partIndex);
                        result.insert(result.end(), r1.begin(), r1.end());
                        
                        // Also try skipping ** at this level
                        auto r2 = expandRecursive(subPath, patternParts, partIndex + 1);
                        result.insert(result.end(), r2.begin(), r2.end());
                    }
                }
            }
        } catch (...) {}
    } else {
        // Regular pattern part
        std::string searchPath = basePath.empty() ? "." : basePath;
        auto matches = expandInDirectory(searchPath, part);
        
        for (const auto& match : matches) {
            auto subResult = expandRecursive(match, patternParts, partIndex + 1);
            result.insert(result.end(), subResult.begin(), subResult.end());
        }
    }
    
    return result;
}

std::vector<std::string> GlobExpander::expand(const std::string& pattern) {
    std::vector<std::string> result;
    
    if (!hasGlobChars(pattern)) {
        // No glob characters, return as-is
        result.push_back(pattern);
        return result;
    }
    
    // Check for ** in pattern
    if (pattern.find("**") != std::string::npos) {
        // Split pattern into parts by /
        std::vector<std::string> parts;
        std::string current;
        for (char c : pattern) {
            if (c == '/' || c == '\\') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            parts.push_back(current);
        }
        
        result = expandRecursive("", parts, 0);
    } else {
        // Simple glob without **
        std::string directory, filename;
        splitPath(pattern, directory, filename);
        
        result = expandInDirectory(directory, filename);
    }
    
    // If no matches, return original pattern
    if (result.empty()) {
        result.push_back(pattern);
    }
    
    return result;
}

std::vector<std::string> GlobExpander::expandTokens(const std::vector<std::string>& tokens) {
    std::vector<std::string> result;
    
    for (const auto& token : tokens) {
        auto expanded = expand(token);
        result.insert(result.end(), expanded.begin(), expanded.end());
    }
    
    return result;
}

} // namespace termidash
