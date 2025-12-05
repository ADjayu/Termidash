#pragma once
#include <string>
#include <vector>
#include <utility>

namespace termidash {

/**
 * @brief Shell command parser utilities
 * 
 * This module handles tokenization, redirection parsing, and batch command splitting.
 */
class Parser {
public:
    /**
     * @brief Trim whitespace from both ends of a string
     */
    static std::string trim(const std::string& s);

    /**
     * @brief Split batch commands by separators (;, &&, ||)
     * @return Vector of pairs: (command, separator after it)
     */
    static std::vector<std::pair<std::string, std::string>> splitBatch(const std::string& input);

    /**
     * @brief Tokenize a command respecting quotes
     */
    static std::vector<std::string> tokenize(const std::string& cmd);

    /**
     * @brief Redirection information parsed from a command
     */
    struct RedirectionInfo {
        std::string command;      // Command without redirection tokens
        std::string inFile;       // Input file for <
        std::string outFile;      // Output file for > or >>
        std::string errFile;      // Error file for 2> or 2>>
        bool appendOut = false;   // True if >> instead of >
        bool appendErr = false;   // True if 2>> instead of 2>
        std::string hereDocDelim; // Delimiter for <<
        bool isHereDoc = false;   // True if << present
    };

    /**
     * @brief Parse redirection tokens from a command
     */
    static RedirectionInfo parseRedirection(const std::string& cmd);

    /**
     * @brief Pipeline segment with trim operator info
     */
    struct PipelineSegment {
        std::string cmd;
        bool trimBeforeNext;
    };

    /**
     * @brief Split a line by pipe operators (| and |>)
     */
    static std::vector<PipelineSegment> splitPipelineOperators(const std::string& line);

    /**
     * @brief Apply trim to each line of input
     */
    static std::string applyTrimToLines(const std::string& input);
};

} // namespace termidash
