#pragma once
#include <string>
#include <vector>

namespace termidash {

/**
 * GlobExpander - Handles shell glob pattern expansion
 * 
 * Patterns:
 *   *     - Match any sequence of characters
 *   ?     - Match single character
 *   [abc] - Match character class
 *   [a-z] - Match character range
 *   **    - Recursive directory match
 */
class GlobExpander {
public:
    /**
     * Expand a glob pattern to matching file paths.
     * @param pattern The glob pattern (e.g., "*.txt")
     * @return Vector of matching file paths, or original pattern if no matches
     */
    static std::vector<std::string> expand(const std::string& pattern);
    
    /**
     * Check if the input contains any glob characters.
     */
    static bool hasGlobChars(const std::string& input);
    
    /**
     * Expand multiple tokens, handling globs in each.
     */
    static std::vector<std::string> expandTokens(const std::vector<std::string>& tokens);
    
private:
    /**
     * Match a pattern against a string.
     * @param pattern The glob pattern
     * @param str The string to match
     * @return true if the pattern matches the string
     */
    static bool matchPattern(const std::string& pattern, const std::string& str);
    
    /**
     * Match a character class pattern like [abc] or [a-z].
     * @return Number of characters consumed from pattern, 0 if no match
     */
    static size_t matchCharClass(const std::string& pattern, size_t pos, char c);
    
    /**
     * Expand glob with recursive ** pattern.
     */
    static std::vector<std::string> expandRecursive(
        const std::string& basePath,
        const std::vector<std::string>& patternParts,
        size_t partIndex);
    
    /**
     * Expand a single glob pattern in a specific directory.
     */
    static std::vector<std::string> expandInDirectory(
        const std::string& directory,
        const std::string& pattern);
    
    /**
     * Split path into directory and pattern parts.
     */
    static void splitPath(const std::string& path, 
        std::string& directory, std::string& filename);
};

} // namespace termidash
