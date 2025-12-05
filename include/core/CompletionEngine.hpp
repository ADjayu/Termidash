#pragma once
#include <string>
#include <vector>
#include <functional>

namespace termidash {

/**
 * @brief Tab completion engine with fuzzy matching
 * 
 * Uses Longest Common Subsequence (LCS) for fuzzy matching and ranking.
 */
class CompletionEngine {
public:
    /**
     * @brief Completion candidate with scoring
     */
    struct Candidate {
        std::string name;
        int score;
    };

    /**
     * @brief Compute LCS length between two strings
     * @return Length of longest common subsequence
     */
    static int lcsLength(const std::string& a, const std::string& b);

    /**
     * @brief Complete a prefix using the provided generator
     * 
     * Ranking: prefix match > substring match > fuzzy (LCS)
     * 
     * @param prefix The prefix to complete
     * @param generator Function that returns completion candidates
     * @return Sorted list of completions
     */
    static std::vector<std::string> complete(
        const std::string& prefix,
        std::function<std::vector<std::string>(const std::string&)> generator
    );
};

} // namespace termidash
