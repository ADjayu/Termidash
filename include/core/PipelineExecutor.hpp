#pragma once
#include "core/Parser.hpp"
#include "core/BuiltInCommandHandler.hpp"
#include "core/CommandExecutor.hpp"
#include "platform/interfaces/IProcessManager.hpp"
#include "platform/interfaces/ITerminal.hpp"
#include <string>
#include <vector>
#include <istream>

namespace termidash {

/**
 * @brief Pipeline execution engine
 * 
 * Handles execution of piped commands, including:
 * - Standard pipes (|)
 * - Trim pipes (|>)
 * - Redirection within pipeline segments
 * - Mixed built-in and external commands
 */
class PipelineExecutor {
public:
    /**
     * @brief Segment information for pipeline execution
     */
    struct SegmentInfo {
        std::string cleanCmd;
        std::string inFile;
        std::string outFile;
        std::string errFile;
        bool appendOut = false;
        bool appendErr = false;
        bool trimBeforeNext = false;
        std::string hereDocDelim;
        bool isHereDoc = false;
    };

    /**
     * @brief Execute a pipeline of commands
     * 
     * @param pipelineLine The full pipeline command string
     * @param builtInHandler Handler for built-in commands
     * @param executor Command executor
     * @param processManager Platform process manager
     * @param inputSource Optional input stream (for here-docs in scripts)
     * @param terminal Optional terminal for interactive here-doc input
     * @return Exit code of the last command in the pipeline
     */
    static int execute(
        const std::string& pipelineLine,
        BuiltInCommandHandler& builtInHandler,
        ICommandExecutor* executor,
        platform::IProcessManager* processManager,
        std::istream* inputSource = nullptr,
        platform::ITerminal* terminal = nullptr
    );

    /**
     * @brief Execute a single command (no pipes)
     * 
     * @param commandLine The command to execute
     * @param builtInHandler Handler for built-in commands
     * @param executor Command executor
     * @param processManager Platform process manager
     * @param inputSource Optional input stream
     * @param terminal Optional terminal
     * @return Exit code
     */
    static int executeSingle(
        const std::string& commandLine,
        BuiltInCommandHandler& builtInHandler,
        ICommandExecutor* executor,
        platform::IProcessManager* processManager,
        std::istream* inputSource = nullptr,
        platform::ITerminal* terminal = nullptr
    );

private:
    /**
     * @brief Execute pipeline of all built-in commands using StreamBridge
     */
    static int executeBuiltInPipeline(
        const std::vector<SegmentInfo>& segments,
        BuiltInCommandHandler& builtInHandler
    );

    /**
     * @brief Execute pipeline with external commands using OS pipes
     */
    static int executeExternalPipeline(
        const std::vector<SegmentInfo>& segments,
        BuiltInCommandHandler& builtInHandler,
        platform::IProcessManager* processManager
    );

    /**
     * @brief Read here-document content
     */
    static std::string readHereDoc(
        const std::string& delimiter,
        std::istream* inputSource,
        platform::ITerminal* terminal
    );
};

} // namespace termidash
