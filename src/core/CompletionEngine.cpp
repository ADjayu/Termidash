#include "core/CompletionEngine.hpp"
#include <algorithm>
#include <unordered_set>

namespace termidash {

static int my_max(int a, int b) { return a > b ? a : b; }

int CompletionEngine::lcsLength(const std::string& a, const std::string& b) {
    size_t n = a.size();
    size_t m = b.size();
    std::vector<int> dp(m + 1, 0);
    std::vector<int> prev(m + 1, 0);
    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[j] = prev[j - 1] + 1;
            } else {
                dp[j] = my_max(prev[j], dp[j - 1]);
            }
        }
        std::swap(dp, prev);
    }
    return prev[m];
}

std::vector<std::string> CompletionEngine::complete(
    const std::string& prefix,
    std::function<std::vector<std::string>(const std::string&)> generator
) {
    std::vector<Candidate> candidates;
    std::vector<std::string> rawMatches = generator(prefix);

    for (const auto& b : rawMatches) {
        int score = 0;
        // Prefix match: highest priority
        if (b.rfind(prefix, 0) == 0) {
            score += 100;
        }
        // Substring match (not prefix): medium priority
        if (b.find(prefix) != std::string::npos && b.rfind(prefix, 0) != 0) {
            score += 50;
        }
        // Fuzzy match via LCS
        int lcs = lcsLength(prefix, b);
        if (lcs > 0) {
            score += lcs;
        }
        if (score > 0) {
            candidates.push_back({b, score});
        }
    }

    // Sort by score descending, then alphabetically
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.name < b.name;
    });

    // Deduplicate and extract names
    std::vector<std::string> matches;
    std::unordered_set<std::string> seen;
    for (const auto& c : candidates) {
        if (seen.insert(c.name).second) {
            matches.push_back(c.name);
        }
    }
    return matches;
}

} // namespace termidash
