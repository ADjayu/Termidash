#pragma once
#include <string>
#include <vector>

namespace termidash {

/**
 * BraceExpander - Handles {a,b,c} and {1..5} brace expansion
 * 
 * Examples:
 *   file{1,2,3}.txt  -> file1.txt file2.txt file3.txt
 *   {a..e}           -> a b c d e
 *   {1..5}           -> 1 2 3 4 5
 *   {a,{b,c}}        -> a b c (nested)
 */
class BraceExpander {
public:
    /**
     * Expand all brace patterns in the input string.
     * @param input The input string containing brace patterns
     * @return Vector of expanded strings
     */
    static std::vector<std::string> expand(const std::string& input);
    
    /**
     * Check if the input contains any brace expansion patterns.
     */
    static bool hasBraces(const std::string& input);
    
private:
    /**
     * Find the matching closing brace, handling nested braces.
     * @return Position of } or std::string::npos if not found
     */
    static size_t findMatchingBrace(const std::string& str, size_t openPos);
    
    /**
     * Expand a single brace expression (comma list or range).
     * @param prefix Text before the brace
     * @param braceContent Content inside the braces (without braces)
     * @param suffix Text after the brace (may contain more braces)
     */
    static std::vector<std::string> expandBraceContent(
        const std::string& prefix,
        const std::string& braceContent,
        const std::string& suffix);
    
    /**
     * Check if content is a range expression (e.g., "1..5" or "a..z").
     */
    static bool isRange(const std::string& content);
    
    /**
     * Expand a range expression.
     * @param rangeSpec The range specification like "1..5" or "a..z"
     */
    static std::vector<std::string> expandRange(const std::string& rangeSpec);
    
    /**
     * Split comma-separated items, respecting nested braces.
     */
    static std::vector<std::string> splitByComma(const std::string& content);
};

} // namespace termidash
