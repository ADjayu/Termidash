#include "core/ShellLoop.hpp"
#include "core/BuiltInCommandHandler.hpp"
#include "core/CommandExecutorFactory.hpp"
#include "core/JobManagerFactory.hpp"
#include "core/SignalHandlerFactory.hpp"
#include "platform/interfaces/IProcessManager.hpp"
#include "platform/interfaces/ITerminal.hpp"
#include "core/Environment.hpp"
#include "core/AliasManager.hpp"
#include "core/VariableManager.hpp"
#include "core/FunctionManager.hpp"
#include "core/ExpressionEvaluator.hpp"
#include "common/PlatformUtils.hpp"
#include <iostream>
#include <fstream>
#include "core/RingBuffer.hpp"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <filesystem>
#include <thread>
#include <thread>
#include <windows.h>
#include <cstdlib>
#include <functional>
#include <sstream>

namespace termidash
{
    struct Block {
        enum Type { If, While, For, Function };
        Type type;
        std::string condition; // For If/While
        std::string loopVar;   // For For loop
        std::vector<std::string> items; // For For loop
        std::vector<std::string> body;
        std::vector<std::string> elseBody;
        bool inElse = false;
    };

    struct ShellState {
        std::vector<Block> blockStack;
        bool inBlock() const { return !blockStack.empty(); }
    };

    // Helper: trim whitespace
    static std::string trim(const std::string &s)
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
            return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    // Split batch commands and separators (;, &&, ||)
    static std::vector<std::pair<std::string, std::string>> splitBatch(const std::string &input)
    {
        std::vector<std::pair<std::string, std::string>> result;
        size_t i = 0;
        std::string current;
        while (i < input.size())
        {
            if (i + 1 < input.size())
            {
                std::string two = input.substr(i, 2);
                if (two == "&&" || two == "||")
                {
                    result.emplace_back(trim(current), two);
                    current.clear();
                    i += 2;
                    continue;
                }
            }
            if (input[i] == ';')
            {
                result.emplace_back(trim(current), ";");
                current.clear();
                i++;
                continue;
            }
            current += input[i++];
        }
        if (!current.empty())
        {
            result.emplace_back(trim(current), "");
        }
        return result;
    }



    static int my_max(int a, int b) { return a > b ? a : b; }

    // compute longest common subsequence length (for fuzzy scoring)
    static int lcs_length(const std::string &a, const std::string &b)
    {
        size_t n = a.size();
        size_t m = b.size();
        std::vector<int> dp(m + 1, 0);
        std::vector<int> prev(m + 1, 0);
        for (size_t i = 1; i <= n; ++i)
        {
            for (size_t j = 1; j <= m; ++j)
            {
                if (a[i - 1] == b[j - 1])
                {
                    dp[j] = prev[j - 1] + 1;
                }
                else
                {
                    dp[j] = my_max(prev[j], dp[j - 1]);
                }
            }
            std::swap(dp, prev);
        }
        return prev[m];
    }

    // Dynamic tab completion with ranking: prefix > substring > fuzzy via LCS
    static std::vector<std::string> completePrefix(const std::string &prefix, std::function<std::vector<std::string>(const std::string&)> generator)
    {
        struct Candidate
        {
            std::string name;
            int score;
        };
        std::vector<Candidate> candidates;

        std::vector<std::string> rawMatches = generator(prefix);

        for (const auto &b : rawMatches)
        {
            int score = 0;
            if (b.rfind(prefix, 0) == 0)
            {
                score += 100;
            }
            if (b.find(prefix) != std::string::npos && b.rfind(prefix, 0) != 0)
            {
                score += 50;
            }
            int lcs = lcs_length(prefix, b);
            if (lcs > 0)
                score += lcs;
            if (score > 0)
                candidates.push_back({b, score});
        }

        std::sort(candidates.begin(), candidates.end(), [](const Candidate &a, const Candidate &b)
                  {
            if (a.score != b.score) return a.score > b.score;
            return a.name < b.name; });

        std::vector<std::string> matches;
        std::unordered_set<std::string> seen;
        for (const auto &c : candidates)
        {
            if (seen.insert(c.name).second)
            {
                matches.push_back(c.name);
            }
        }
        return matches;
    }

    // Custom pipeline parsing with "|>" trim operator
    struct PipelineSegment
    {
        std::string cmd;
        bool trimBeforeNext;
    };

    static std::vector<PipelineSegment> splitPipelineOperators(const std::string &line)
    {
        std::vector<PipelineSegment> segments;
        size_t pos = 0;
        std::string current;
        while (pos < line.size())
        {
            if (pos + 1 < line.size() && line.substr(pos, 2) == "|>")
            {
                segments.push_back({trim(current), true});
                current.clear();
                pos += 2;
            }
            else if (line[pos] == '|')
            {
                segments.push_back({trim(current), false});
                current.clear();
                pos++;
            }
            else
            {
                current += line[pos++];
            }
        }
        if (!current.empty())
        {
            segments.push_back({trim(current), false});
        }
        return segments;
    }

    static std::string applyTrimToLines(const std::string &input)
    {
        std::string out;
        std::string line;
        size_t pos = 0;
        size_t start = 0;
        while (start < input.size())
        {
            size_t npos = input.find('\n', start);
            if (npos == std::string::npos)
                npos = input.size();
            line = input.substr(start, npos - start);
            size_t s = line.find_first_not_of(" 	");
            size_t e = line.find_last_not_of(" 	");
            if (s != std::string::npos && e != std::string::npos && e >= s)
            {
                out += line.substr(s, e - s + 1);
                out += '\n';
            }
            start = (npos == input.size()) ? npos : npos + 1;
        }
        return out;
    }

    // Read a line with history navigation (up/down) and tab completion
    static std::string readLineInteractive(platform::ITerminal* terminal, const std::vector<std::string> &history, size_t &history_index, std::function<std::vector<std::string>(const std::string&)> completionGenerator)
    {
        std::string buffer;
        size_t cursor = 0;
        while (true)
        {
            int ch = terminal->readChar();
            if (ch == 13)
            { // Enter
                terminal->write("\n");
                break;
            }
            else if (ch == 8)
            { // Backspace
                if (cursor > 0)
                {
                    buffer.erase(cursor - 1, 1);
                    cursor--;
                    terminal->write("\b \b");
                }
            }
            else if (ch == 9)
            { // Tab
                size_t pos = buffer.find_last_of(" \t");
                std::string prefix = (pos == std::string::npos) ? buffer : buffer.substr(pos + 1);
                auto matches = completePrefix(prefix, completionGenerator);
                if (matches.size() == 1)
                {
                    std::string addition = matches[0].substr(prefix.size());
                    buffer += addition;
                    terminal->write(addition);
                    cursor += addition.size();
                }
                else if (matches.size() > 1)
                {
                    terminal->write("\n");
                    for (size_t i = 0; i < matches.size() && i < 10; ++i)
                    {
                        terminal->write(matches[i] + " ");
                    }
                    terminal->write("\n> " + buffer);
                }
            }
            else if (ch == 224)
            { // arrows
                int next = terminal->readChar();
                if (next == 72)
                { // up
                    if (history_index > 0)
                    {
                        history_index--;
                        for (size_t i = 0; i < buffer.size(); ++i)
                            terminal->write("\b \b");
                        buffer = history[history_index];
                        cursor = buffer.size();
                        terminal->write(buffer);
                    }
                }
                else if (next == 80)
                { // down
                    if (history_index + 1 < history.size())
                    {
                        history_index++;
                        for (size_t i = 0; i < buffer.size(); ++i)
                            terminal->write("\b \b");
                        buffer = history[history_index];
                        cursor = buffer.size();
                        terminal->write(buffer);
                    }
                    else
                    {
                        for (size_t i = 0; i < buffer.size(); ++i)
                            terminal->write("\b \b");
                        buffer.clear();
                        cursor = 0;
                    }
                }
            }
            else if (isprint(ch))
            {
                buffer.insert(buffer.begin() + cursor, static_cast<char>(ch));
                terminal->write(std::string(1, static_cast<char>(ch)));
                cursor++;
            }
        }
        history_index = history.size();
        return buffer;
    }

    // Parse redirection tokens
    static void parseRedirection(const std::string &cmd, std::string &outCommand, std::string &inFile, std::string &outFile, std::string &errFile, bool &appendOut, bool &appendErr, std::string &hereDocDelim, bool &isHereDoc)
    {
        appendOut = false;
        appendErr = false;
        isHereDoc = false;
        outCommand.clear();
        inFile.clear();
        outFile.clear();
        errFile.clear();
        hereDocDelim.clear();

        std::vector<std::string> toks;
        // split respecting quotes
        std::string cur;
        bool inQuotes = false;
        for (size_t i = 0; i < cmd.size(); ++i)
        {
            char c = cmd[i];
            if (c == '\"')
            {
                inQuotes = !inQuotes;
                cur.push_back(c); // Keep quotes for now, handle later or let command handler handle it
                continue;
            }
            if (!inQuotes && (c == ' ' || c == '\t'))
            {
                if (!cur.empty())
                {
                    toks.push_back(cur);
                    cur.clear();
                }
            }
            else
            {
                cur.push_back(c);
            }
        }
        if (!cur.empty())
            toks.push_back(cur);

        for (size_t i = 0; i < toks.size(); ++i)
        {
            const std::string &t = toks[i];
            if (t == "<")
            {
                if (i + 1 < toks.size())
                    inFile = toks[i + 1], ++i;
            }
            else if (t == "<<")
            {
                if (i + 1 < toks.size()) {
                    hereDocDelim = toks[i + 1];
                    isHereDoc = true;
                    ++i;
                }
            }
            else if (t == ">>")
            {
                if (i + 1 < toks.size())
                    outFile = toks[i + 1], appendOut = true, ++i;
            }
            else if (t == ">" || t == "1>")
            {
                if (i + 1 < toks.size())
                    outFile = toks[i + 1], appendOut = false, ++i;
            }
            else if (t == "2>")
            {
                if (i + 1 < toks.size())
                    errFile = toks[i + 1], appendErr = false, ++i;
            }
            else if (t == "2>>")
            {
                if (i + 1 < toks.size())
                    errFile = toks[i + 1], appendErr = true, ++i;
            }
            else if (t == "&>" || t == ">&")
            {
                if (i + 1 < toks.size()) {
                    outFile = toks[i + 1];
                    errFile = toks[i + 1];
                    appendOut = false;
                    appendErr = false;
                    ++i;
                }
            }
            else if (t == "&>>")
            {
                if (i + 1 < toks.size()) {
                    outFile = toks[i + 1];
                    errFile = toks[i + 1];
                    appendOut = true;
                    appendErr = true;
                    ++i;
                }
            }
            else
            {
                if (!outCommand.empty())
                    outCommand += ' ';
                outCommand += t;
            }
        }
    }

    // Execute a single (non-pipeline) command, considering redirection and built-ins
    static int executeSingle(const std::string &commandLine, BuiltInCommandHandler &builtInHandler, ICommandExecutor *executor, platform::IProcessManager* processManager, std::istream* inputSource = nullptr, platform::ITerminal* terminal = nullptr)
    {
        std::string trimmed = trim(commandLine);
        if (trimmed.empty())
            return 0;

        std::string cleanCmd, inFile, outFile, errFile;
        bool appendOut = false;
        bool appendErr = false;
        std::string hereDocDelim;
        bool isHereDoc = false;
        parseRedirection(trimmed, cleanCmd, inFile, outFile, errFile, appendOut, appendErr, hereDocDelim, isHereDoc);

        if (isHereDoc) {
            std::string tempFile = ".heredoc_" + std::to_string(std::rand());
            std::ofstream tempOut(tempFile);
            if (!tempOut.is_open()) {
                std::cerr << "Error: Cannot create temporary file for here-document\n";
                return 1;
            }

            std::string line;
            std::vector<std::string> dummyHist;
            size_t dummyIdx = 0;
            auto dummyGen = [](const std::string&) { return std::vector<std::string>{}; };
            while (true) {
                if (inputSource) {
                    if (!std::getline(*inputSource, line)) break;
                } else if (terminal) {
                    terminal->write("> ");
                    line = readLineInteractive(terminal, dummyHist, dummyIdx, dummyGen);
                } else {
                    break; 
                }
                if (trim(line) == hereDocDelim) break;
                tempOut << line << "\n";
            }
            tempOut.close();
            inFile = tempFile;
        }

        // Check if it is a built-in command
        std::string cmdName;
        size_t spacePos = cleanCmd.find(' ');
        if (spacePos != std::string::npos)
            cmdName = cleanCmd.substr(0, spacePos);
        else
            cmdName = cleanCmd;

        if (builtInHandler.isBuiltInCommand(cmdName)) {
            std::ifstream inFileStream;
            std::ofstream outFileStream;
            std::ofstream errFileStream;
            std::istream* inPtr = &std::cin;
            std::ostream* outPtr = &std::cout;
            std::ostream* errPtr = &std::cerr;

            if (!inFile.empty()) {
                inFileStream.open(inFile);
                if (!inFileStream.is_open()) {
                    std::cerr << "Error: Cannot open input file: " << inFile << "\n";
                    return 1;
                }
                inPtr = &inFileStream;
            }

            if (!outFile.empty()) {
                outFileStream.open(outFile, appendOut ? std::ios::app : std::ios::out);
                if (!outFileStream.is_open()) {
                    std::cerr << "Error: Cannot open output file: " << outFile << "\n";
                    return 1;
                }
                outPtr = &outFileStream;
            }

            if (!errFile.empty()) {
                if (errFile == outFile && outPtr == &outFileStream) {
                    errPtr = outPtr;
                } else {
                    errFileStream.open(errFile, appendErr ? std::ios::app : std::ios::out);
                    if (!errFileStream.is_open()) {
                        std::cerr << "Error: Cannot open error file: " << errFile << "\n";
                        return 1;
                    }
                    errPtr = &errFileStream;
                }
            }

            ExecContext ctx(*inPtr, *outPtr, *errPtr);
            int result = builtInHandler.handleCommandWithContext(cleanCmd, ctx);
            return result;
        }

        // External command
        long stdIn = -1;
        long stdOut = -1;
        long stdErr = -1;

        if (!inFile.empty()) {
            stdIn = PlatformUtils::openFileForRead(inFile);
            if (stdIn == -1) {
                std::cerr << "Error: Cannot open input file: " << inFile << "\n";
                return 1;
            }
        }

        if (!outFile.empty()) {
            stdOut = PlatformUtils::openFileForWrite(outFile, appendOut);
            if (stdOut == -1) {
                std::cerr << "Error: Cannot open output file: " << outFile << "\n";
                if (stdIn != -1) PlatformUtils::closeFile(stdIn);
                return 1;
            }
        }

        if (!errFile.empty()) {
            if (errFile == outFile && stdOut != -1) {
                // Duplicate handle for stderr if it's the same file
                // On Windows, we can just use the same handle value for spawn if we want, 
                // OR we can duplicate it. IProcessManager::spawn takes separate args.
                // If we pass the same handle value, it should work as long as we don't close it twice.
                // But wait, executeSingle closes handles at the end.
                // If we pass the same handle, we must be careful not to close it twice.
                // Actually, PlatformUtils::openFileForWrite returns a NEW handle.
                // If we want to write to the same file, we should probably duplicate the handle 
                // or just pass the same handle to spawn and manage closing carefully.
                // Simplest way: pass stdOut to stdErr in spawn, and set stdErr = -1 so we don't close it.
                // But spawn signature is (..., stdIn, stdOut, stdErr).
                // If I pass stdOut as stdErr, spawn will use it.
                // Then I close stdOut. I should NOT close stdErr if it's the same.
                stdErr = stdOut; 
            } else {
                stdErr = PlatformUtils::openFileForWrite(errFile, appendErr);
                if (stdErr == -1) {
                    std::cerr << "Error: Cannot open error file: " << errFile << "\n";
                    if (stdIn != -1) PlatformUtils::closeFile(stdIn);
                    if (stdOut != -1) PlatformUtils::closeFile(stdOut);
                    return 1;
                }
            }
        }

        // Parse command args
        std::vector<std::string> tokens;
        std::string token;
        bool inQuotes = false;
        for (char c : cleanCmd) {
            if (c == '\"') inQuotes = !inQuotes;
            else if (c == ' ' && !inQuotes) {
                if (!token.empty()) { tokens.push_back(token); token.clear(); }
            } else token += c;
        }
        if (!token.empty()) tokens.push_back(token);

        if (tokens.empty()) {
             if (stdIn != -1) PlatformUtils::closeFile(stdIn);
             if (stdOut != -1) PlatformUtils::closeFile(stdOut);
             if (stdErr != -1 && stdErr != stdOut) PlatformUtils::closeFile(stdErr);
             return 0;
        }

        std::string cmd = tokens[0];
        std::vector<std::string> args;
        if (tokens.size() > 1) args.assign(tokens.begin() + 1, tokens.end());

        long pid = processManager->spawn(cmd, args, false, stdIn, stdOut, stdErr);
        
        if (stdIn != -1) PlatformUtils::closeFile(stdIn);
        if (stdOut != -1) PlatformUtils::closeFile(stdOut);
        if (stdErr != -1 && stdErr != stdOut) PlatformUtils::closeFile(stdErr);

        if (pid == -1)
        {
            std::cerr << "Error: Failed to spawn: " << cmd << " Error: " << processManager->getLastError() << "\n";
            return 1;
        }

        int exitCode = processManager->wait(pid);
        return exitCode;
    }

    static int executePipeline(const std::string &pipelineLine, BuiltInCommandHandler &builtInHandler, ICommandExecutor *executor, platform::IProcessManager* processManager, std::istream* inputSource = nullptr, platform::ITerminal* terminal = nullptr)
    {
        auto rawSegments = splitPipelineOperators(pipelineLine);
        if (rawSegments.empty())
            return 0;

        struct SegmentInfo {
            std::string cleanCmd;
            std::string inFile;
            std::string outFile;
            std::string errFile;
            bool appendOut;
            bool appendErr;
            bool trimBeforeNext;
            std::string hereDocDelim;
            bool isHereDoc;
        };

        std::vector<SegmentInfo> segments;
        bool allBuiltIn = true;

        for (const auto& raw : rawSegments) {
            SegmentInfo info;
            info.trimBeforeNext = raw.trimBeforeNext;
            parseRedirection(raw.cmd, info.cleanCmd, info.inFile, info.outFile, info.errFile, info.appendOut, info.appendErr, info.hereDocDelim, info.isHereDoc);
            
            if (info.isHereDoc) {
                std::string tempFile = ".heredoc_" + std::to_string(std::rand());
                std::ofstream tempOut(tempFile);
                if (tempOut.is_open()) {
                    std::string line;
                    std::vector<std::string> dummyHist;
                    size_t dummyIdx = 0;
                    auto dummyGen = [](const std::string&) { return std::vector<std::string>{}; };
                    while (true) {
                        if (inputSource) {
                            if (!std::getline(*inputSource, line)) break;
                        } else if (terminal) {
                            terminal->write("> ");
                            line = readLineInteractive(terminal, dummyHist, dummyIdx, dummyGen);
                        } else {
                            break; 
                        }
                        if (trim(line) == info.hereDocDelim) break;
                        tempOut << line << "\n";
                    }
                    tempOut.close();
                    info.inFile = tempFile;
                } else {
                     std::cerr << "Error: Cannot create temporary file for here-document\n";
                }
            }

            segments.push_back(info);

            std::string cmdName;
            size_t spacePos = info.cleanCmd.find(' ');
            if (spacePos != std::string::npos)
                cmdName = info.cleanCmd.substr(0, spacePos);
            else
                cmdName = info.cleanCmd;

            if (!builtInHandler.isBuiltInCommand(cmdName)) {
                allBuiltIn = false;
            }
        }

        if (allBuiltIn)
        {
            size_t n = segments.size();
            std::vector<std::shared_ptr<termidash::StreamBridge>> bridges;
            for (size_t i = 0; i < n - 1; ++i)
            {
                bridges.push_back(std::make_shared<termidash::StreamBridge>());
            }

            std::vector<std::thread> threads;
            threads.reserve(n);
            std::vector<int> exitCodes(n, 0);

            for (size_t i = 0; i < n; ++i)
            {
                SegmentInfo info = segments[i];
                std::shared_ptr<termidash::StreamBridge> prevBridge = (i > 0) ? bridges[i - 1] : nullptr;
                std::shared_ptr<termidash::StreamBridge> nextBridge = (i < n - 1) ? bridges[i] : nullptr;

                std::thread th([info, prevBridge, nextBridge, &builtInHandler, &exitCodes, i]()
                {
                    std::ifstream inFileStream;
                    std::ofstream outFileStream;
                    std::ofstream errFileStream;
                    std::istream* inPtr = nullptr;
                    std::ostream* outPtr = nullptr;
                    std::ostream* errPtr = &std::cerr;

                    if (!info.inFile.empty()) {
                        inFileStream.open(info.inFile);
                        if (inFileStream.is_open()) inPtr = &inFileStream;
                        else std::cerr << "Error: Cannot open input file: " << info.inFile << "\n";
                    } else if (prevBridge) {
                        inPtr = &prevBridge->in();
                    } else {
                        inPtr = &std::cin;
                    }

                    if (!info.outFile.empty()) {
                        outFileStream.open(info.outFile, info.appendOut ? std::ios::app : std::ios::out);
                        if (outFileStream.is_open()) outPtr = &outFileStream;
                        else std::cerr << "Error: Cannot open output file: " << info.outFile << "\n";
                    } else if (nextBridge) {
                        outPtr = &nextBridge->out();
                    } else {
                        outPtr = &std::cout;
                    }

                    if (!info.errFile.empty()) {
                        if (info.errFile == info.outFile && outPtr == &outFileStream) {
                            errPtr = outPtr;
                        } else {
                            errFileStream.open(info.errFile, info.appendErr ? std::ios::app : std::ios::out);
                            if (errFileStream.is_open()) errPtr = &errFileStream;
                            else std::cerr << "Error: Cannot open error file: " << info.errFile << "\n";
                        }
                    }

                    if (inPtr && outPtr) {
                        ExecContext ctx(*inPtr, *outPtr, *errPtr);
                        exitCodes[i] = builtInHandler.handleCommandWithContext(info.cleanCmd, ctx);
                    }
                    
                    if (nextBridge) nextBridge->closeWriter();
                });
                threads.push_back(std::move(th));
            }

            for (auto& th : threads) {
                if (th.joinable()) th.join();
            }

            return exitCodes.back(); 
        }
        else
        {
            size_t n = segments.size();
            std::vector<long> pids;
            long prevRead = -1;

            for (size_t i = 0; i < n; ++i)
            {
                long nextRead = -1;
                long nextWrite = -1;

                if (i < n - 1)
                {
                    if (!processManager->createPipe(nextRead, nextWrite))
                    {
                        std::cerr << "Failed to create pipe: " << processManager->getLastError() << "\n";
                        return 1;
                    }
                }

                long stdIn = -1;
                long stdOut = -1;
                long stdErr = -1;

                if (!segments[i].inFile.empty()) {
                    stdIn = PlatformUtils::openFileForRead(segments[i].inFile);
                    if (stdIn == -1) std::cerr << "Error: Cannot open input file: " << segments[i].inFile << "\n";
                } else if (i > 0) {
                    stdIn = prevRead;
                } else {
                    stdIn = -1; 
                }

                if (!segments[i].outFile.empty()) {
                    stdOut = PlatformUtils::openFileForWrite(segments[i].outFile, segments[i].appendOut);
                    if (stdOut == -1) std::cerr << "Error: Cannot open output file: " << segments[i].outFile << "\n";
                } else if (i < n - 1) {
                    stdOut = nextWrite;
                } else {
                    stdOut = -1;
                }

                if (!segments[i].errFile.empty()) {
                     if (segments[i].errFile == segments[i].outFile && stdOut != -1) {
                         stdErr = stdOut;
                     } else {
                         stdErr = PlatformUtils::openFileForWrite(segments[i].errFile, segments[i].appendErr);
                         if (stdErr == -1) std::cerr << "Error: Cannot open error file: " << segments[i].errFile << "\n";
                     }
                }

                std::vector<std::string> tokens;
                std::string token;
                bool inQuotes = false;
                for (char c : segments[i].cleanCmd) {
                    if (c == '\"') inQuotes = !inQuotes;
                    else if (c == ' ' && !inQuotes) {
                        if (!token.empty()) { tokens.push_back(token); token.clear(); }
                    } else token += c;
                }
                if (!token.empty()) tokens.push_back(token);

                if (tokens.empty()) {
                    if (stdIn != -1 && stdIn != prevRead) PlatformUtils::closeFile(stdIn);
                    if (stdOut != -1 && stdOut != nextWrite) PlatformUtils::closeFile(stdOut);
                    if (stdErr != -1 && stdErr != stdOut) PlatformUtils::closeFile(stdErr);
                    if (prevRead != -1) processManager->closeHandle(prevRead);
                    if (nextWrite != -1) processManager->closeHandle(nextWrite);
                    if (nextRead != -1) processManager->closeHandle(nextRead);
                    continue; 
                }

                std::string cmd = tokens[0];
                std::vector<std::string> args;
                if (tokens.size() > 1) args.assign(tokens.begin() + 1, tokens.end());

                long pid = processManager->spawn(cmd, args, false, stdIn, stdOut, stdErr);
                
                if (!segments[i].inFile.empty() && stdIn != -1) PlatformUtils::closeFile(stdIn);
                if (!segments[i].outFile.empty() && stdOut != -1) PlatformUtils::closeFile(stdOut);
                if (!segments[i].errFile.empty() && stdErr != -1 && stdErr != stdOut) PlatformUtils::closeFile(stdErr);

                if (pid == -1)
                {
                    std::cerr << "Failed to spawn: " << cmd << " Error: " << processManager->getLastError() << "\n";
                    if (prevRead != -1) processManager->closeHandle(prevRead);
                    if (nextWrite != -1) processManager->closeHandle(nextWrite);
                    if (nextRead != -1) processManager->closeHandle(nextRead);
                    return 1;
                }
                pids.push_back(pid);

                if (prevRead != -1) processManager->closeHandle(prevRead);
                if (nextWrite != -1) processManager->closeHandle(nextWrite);

                prevRead = nextRead;
            }

            int lastExitCode = 0;
            for (long pid : pids)
            {
                int code = processManager->wait(pid);
                if (pid == pids.back()) lastExitCode = code;
            }

            return lastExitCode;
        }
    }

    // Helper: Expand variables and aliases
    static std::string expandString(const std::string& input) {
        std::string cmd = input;
        // Variable expansion
        std::string expandedVarsCmd;
        for (size_t i = 0; i < cmd.size(); ++i) {
            // Check for arithmetic expansion $((...))
            if (cmd.size() > i + 3 && cmd[i] == '$' && cmd[i+1] == '(' && cmd[i+2] == '(') {
                size_t start = i + 3;
                size_t end = cmd.find("))", start);
                if (end != std::string::npos) {
                    std::string expr = cmd.substr(start, end - start);
                    // Recursively expand variables inside expression
                    expr = expandString(expr); 
                    try {
                        long long result = ExpressionEvaluator::evaluate(expr);
                        expandedVarsCmd += std::to_string(result);
                        i = end + 1;
                        continue;
                    } catch (const std::exception& e) {
                        std::cerr << "Arithmetic error: " << e.what() << "\n";
                        // Fallback: keep original
                        expandedVarsCmd += "$((" + expr + "))";
                        i = end + 1;
                        continue;
                    }
                }
            }

            if (cmd[i] == '$') {
                size_t j = i + 1;
                std::string varName;
                while (j < cmd.size() && (isalnum(cmd[j]) || cmd[j] == '_')) {
                    varName += cmd[j];
                    j++;
                }
                if (!varName.empty()) {
                    expandedVarsCmd += VariableManager::instance().get(varName);
                    i = j - 1;
                } else {
                    expandedVarsCmd += '$';
                }
            } else {
                expandedVarsCmd += cmd[i];
            }
        }
        cmd = expandedVarsCmd;

        // Alias expansion (only at start)
        std::string expandedCmd = cmd;
        std::string cmdName;
        size_t spacePos = cmd.find(' ');
        if (spacePos != std::string::npos)
            cmdName = cmd.substr(0, spacePos);
        else
            cmdName = cmd;

        if (AliasManager::instance().has(cmdName)) {
            std::string aliasVal = AliasManager::instance().get(cmdName);
            if (spacePos != std::string::npos) {
                expandedCmd = aliasVal + cmd.substr(spacePos);
            } else {
                expandedCmd = aliasVal;
            }
        }
        return expandedCmd;
    }

    static void processInputLine(const std::string &input, BuiltInCommandHandler &builtInHandler, ICommandExecutor *executor, platform::IProcessManager* processManager, IJobManager *jobManager, ShellState &state, std::istream* inputSource = nullptr, platform::ITerminal* terminal = nullptr)
    {
        auto batches = splitBatch(input);
        int lastExitCode = 0;

        for (const auto &batch : batches)
        {
            std::string cmd = batch.first;
            std::string sep = batch.second;

            if (cmd.empty())
                continue;

            // Check for function definition
            if (cmd.rfind("function ", 0) == 0) {
                std::string name = trim(cmd.substr(9));
                size_t bracePos = name.find('{');
                if (bracePos != std::string::npos) {
                    name = trim(name.substr(0, bracePos));
                }
                Block b;
                b.type = Block::Function;
                b.condition = name; // Store function name in condition field
                state.blockStack.push_back(b);
                continue;
            } else {
                size_t parenPos = cmd.find("()");
                if (parenPos != std::string::npos) {
                    std::string name = trim(cmd.substr(0, parenPos));
                    size_t bracePos = cmd.find('{');
                    if (bracePos != std::string::npos) {
                        Block b;
                        b.type = Block::Function;
                        b.condition = name;
                        state.blockStack.push_back(b);
                        continue;
                    }
                }
            }

            // Check for control flow keywords (on raw command)
            if (cmd.rfind("if ", 0) == 0)
            {
                Block b;
                b.type = Block::If;
                b.condition = cmd.substr(3); // Store raw condition
                state.blockStack.push_back(b);
                continue;
            }
            else if (cmd.rfind("while ", 0) == 0)
            {
                Block b;
                b.type = Block::While;
                b.condition = cmd.substr(6); // Store raw condition
                state.blockStack.push_back(b);
                continue;
            }
            else if (cmd.rfind("for ", 0) == 0)
            {
                Block b;
                b.type = Block::For;
                // for var in item1 item2 ...
                std::string rest = cmd.substr(4);
                size_t inPos = rest.find(" in ");
                if (inPos != std::string::npos)
                {
                    b.loopVar = trim(rest.substr(0, inPos));
                    std::string itemsStr = rest.substr(inPos + 4);
                    // Expand items string immediately
                    itemsStr = expandString(itemsStr);
                    
                    // split items
                    size_t p = 0;
                    std::string it;
                    while (p < itemsStr.size())
                    {
                        if (itemsStr[p] == ' ')
                        {
                            if (!it.empty())
                            {
                                b.items.push_back(it);
                                it.clear();
                            }
                        }
                        else
                        {
                            it += itemsStr[p];
                        }
                        p++;
                    }
                    if (!it.empty())
                        b.items.push_back(it);
                }
                state.blockStack.push_back(b);
                continue;
            }
            else if (cmd == "else")
            {
                if (!state.blockStack.empty() && state.blockStack.back().type == Block::If)
                {
                    state.blockStack.back().inElse = true;
                }
                else
                {
                    std::cerr << "Error: else without if\n";
                }
                continue;
            }
            else if (cmd == "end" || cmd == "}")
            {
                if (!state.blockStack.empty())
                {
                    Block b = state.blockStack.back();
                    state.blockStack.pop_back();

                    if (b.type == Block::Function) {
                        FunctionManager::instance().define(b.condition, b.body);
                    }
                    else if (b.type == Block::If)
                    {
                        // Execute condition
                        std::string condCmd = expandString(b.condition);
                        int res = 0;
                        if (condCmd.find('|') != std::string::npos)
                            res = executePipeline(condCmd, builtInHandler, executor, processManager);
                        else
                            res = executeSingle(condCmd, builtInHandler, executor, processManager);

                        if (res == 0)
                        {
                            for (const auto &line : b.body)
                                processInputLine(line, builtInHandler, executor, processManager, jobManager, state);
                        }
                        else
                        {
                            for (const auto &line : b.elseBody)
                                processInputLine(line, builtInHandler, executor, processManager, jobManager, state);
                        }
                    }
                    else if (b.type == Block::While)
                    {
                        int maxIter = 10000; // Safety limit
                        while (maxIter-- > 0)
                        {
                             std::string condCmd = expandString(b.condition);
                             int res = 0;
                             if (condCmd.find('|') != std::string::npos)
                                res = executePipeline(condCmd, builtInHandler, executor, processManager);
                             else
                                res = executeSingle(condCmd, builtInHandler, executor, processManager);
                             
                             if (res != 0) break;

                             for (const auto &line : b.body)
                                processInputLine(line, builtInHandler, executor, processManager, jobManager, state);
                        }
                    }
                    else if (b.type == Block::For)
                    {
                        for (const auto &item : b.items)
                        {
                            VariableManager::instance().set(b.loopVar, item);
                            for (const auto &line : b.body)
                                processInputLine(line, builtInHandler, executor, processManager, jobManager, state);
                        }
                    }
                }
                else
                {
                    if (cmd == "end") std::cerr << "Error: end without block\n";
                    // } might be part of a command, but here we treat it as block end if matched
                }
                continue;
            }

            if (state.inBlock())
            {
                if (state.blockStack.back().inElse)
                    state.blockStack.back().elseBody.push_back(cmd);
                else
                    state.blockStack.back().body.push_back(cmd);
                continue;
            }

            // Expansion
            cmd = expandString(cmd);

            // Variable assignment (VAR=value)
            size_t eqPos = cmd.find('=');
            if (eqPos != std::string::npos && eqPos > 0) {
                std::string varName = cmd.substr(0, eqPos);
                bool isValidVar = true;
                for (char c : varName) {
                    if (!isalnum(c) && c != '_') {
                        isValidVar = false;
                        break;
                    }
                }
                if (isValidVar) {
                    std::string val = cmd.substr(eqPos + 1);
                    VariableManager::instance().set(varName, val);
                    continue; // Skip execution for assignment
                }
            }

            // Check for function call
            std::string funcName;
            size_t spacePos = cmd.find(' ');
            if (spacePos != std::string::npos)
                funcName = cmd.substr(0, spacePos);
            else
                funcName = cmd;

            if (FunctionManager::instance().has(funcName)) {
                std::vector<std::string> args;
                if (spacePos != std::string::npos) {
                    std::string argStr = cmd.substr(spacePos + 1);
                    // simple split
                    size_t p = 0;
                    std::string arg;
                    bool inQ = false;
                    for (char c : argStr) {
                        if (c == '\"') inQ = !inQ;
                        else if (c == ' ' && !inQ) {
                            if (!arg.empty()) { args.push_back(arg); arg.clear(); }
                        } else arg += c;
                    }
                    if (!arg.empty()) args.push_back(arg);
                }

                VariableManager::instance().pushScope();
                for (size_t i = 0; i < args.size(); ++i) {
                    VariableManager::instance().set(std::to_string(i + 1), args[i]);
                }
                
                const auto& body = FunctionManager::instance().getBody(funcName);
                for (const auto& line : body) {
                    processInputLine(line, builtInHandler, executor, processManager, jobManager, state);
                }
                
                VariableManager::instance().popScope();
                continue;
            }

            // Check for arithmetic command ((...))
            if (cmd.size() >= 4 && cmd.substr(0, 2) == "((" && cmd.substr(cmd.size() - 2) == "))") {
                std::string expr = cmd.substr(2, cmd.size() - 4);
                try {
                    long long result = ExpressionEvaluator::evaluate(expr);
                    lastExitCode = (result != 0) ? 0 : 1;
                } catch (const std::exception& e) {
                    std::cerr << "Arithmetic error: " << e.what() << "\n";
                    lastExitCode = 1;
                }
                
                if (sep == "&&" && lastExitCode != 0)
                    break;
                if (sep == "||" && lastExitCode == 0)
                    break;
                continue;
            }

            // Check for background job
            bool background = false;
            if (!cmd.empty() && cmd.back() == '&') {
                background = true;
                cmd.pop_back();
                while (!cmd.empty() && cmd.back() == ' ') cmd.pop_back();
            }

            // Check for job control commands
            if (cmd == "jobs") {
                auto jobs = jobManager->listJobs();
                for (const auto& job : jobs) {
                    std::string info = "[" + std::to_string(job.jobId) + "] " + std::to_string(job.pid) + " " + job.status + " " + job.command + "\n";
                    // Use builtInHandler context or just cout? processInputLine doesn't have ctx.
                    // But we have executor/processManager.
                    // Let's use std::cout for now as we are in the shell loop.
                    std::cout << info;
                }
                lastExitCode = 0;
                continue;
            }
            else if (cmd.rfind("fg", 0) == 0) {
                std::string arg = trim(cmd.substr(2));
                int jobId = -1;
                if (!arg.empty()) {
                    if (arg[0] == '%') arg = arg.substr(1);
                    try {
                        jobId = std::stoi(arg);
                    } catch (...) {}
                }
                
                if (jobId != -1) {
                    if (jobManager->bringToForeground(jobId)) {
                        lastExitCode = 0;
                    } else {
                        std::cerr << "fg: job not found: " << jobId << "\n";
                        lastExitCode = 1;
                    }
                } else {
                    std::cerr << "fg: usage: fg %job_id\n";
                    lastExitCode = 1;
                }
                continue;
            }
            else if (cmd.rfind("bg", 0) == 0) {
                std::string arg = trim(cmd.substr(2));
                int jobId = -1;
                if (!arg.empty()) {
                    if (arg[0] == '%') arg = arg.substr(1);
                    try {
                        jobId = std::stoi(arg);
                    } catch (...) {}
                }

                if (jobId != -1) {
                    if (jobManager->continueInBackground(jobId)) {
                        lastExitCode = 0;
                    } else {
                        std::cerr << "bg: job not found: " << jobId << "\n";
                        lastExitCode = 1;
                    }
                } else {
                    std::cerr << "bg: usage: bg %job_id\n";
                    lastExitCode = 1;
                }
                continue;
            }

            if (background) {
                int jobId = jobManager->startJob(cmd);
                if (jobId != -1) {
                    std::cout << "[" << jobId << "] " << cmd << "\n";
                    lastExitCode = 0;
                } else {
                    lastExitCode = 1;
                }
                continue;
            }

            // Normal execution
            if (cmd.find('|') != std::string::npos)
            {
                lastExitCode = executePipeline(cmd, builtInHandler, executor, processManager, inputSource, terminal);
            }
            else
            {
                lastExitCode = executeSingle(cmd, builtInHandler, executor, processManager, inputSource, terminal);
            }

            if (sep == "&&" && lastExitCode != 0)
                break;
            if (sep == "||" && lastExitCode == 0)
                break;
        }
    }

    void runShell(platform::ITerminal* terminal, platform::IProcessManager* processManager)
    {
        auto executorUP = createCommandExecutor(); // unique_ptr<ICommandExecutor>
        ICommandExecutor *executor = executorUP.get();
        auto jobManager = createJobManager();
        auto signalHandler = createSignalHandler();
        signalHandler->setupHandlers();

        BuiltInCommandHandler builtInHandler;

        std::vector<std::string> history;
        size_t history_index = 0;

        // Load history
        std::string historyPath = PlatformUtils::getHistoryFilePath();
        std::ifstream histFile(historyPath);
        if (histFile.is_open()) {
            std::string line;
            while (std::getline(histFile, line)) {
                if (!line.empty()) history.push_back(line);
            }
        }
        history_index = history.size();

        history_index = history.size();
        
        // Dynamic completion generator
        auto completionGenerator = [](const std::string& prefix) -> std::vector<std::string> {
            std::vector<std::string> matches;
            
            // 1. Built-in commands
            static const std::vector<std::string> builtins = {
                "cd", "cls", "ver", "getenv", "setenv", "cwd", "drives", "type", "mkdir", "rmdir", "copy", "del",
                "tasklist", "taskkill", "ping", "ipconfig", "whoami", "hostname", "assoc", "systeminfo", "netstat",
                "echo", "pause", "time", "date", "dir", "attrib", "help", "clear", "exit", "version", "alias", "unalias",
                "pwd", "touch", "rm", "cat", "uptime", "history", "grep", "sort", "head", "tail", "jobs", "fg", "bg", "source",
                "if", "else", "while", "for", "end", "unset", "function"
            };
            for (const auto& cmd : builtins) {
                if (cmd.find(prefix) == 0) matches.push_back(cmd);
            }

            // 2. Executables in PATH (only if prefix doesn't look like a path)
            if (prefix.find('/') == std::string::npos && prefix.find('\\') == std::string::npos) {
                std::string pathEnv = PlatformUtils::getEnv("PATH");
                char sep = PlatformUtils::getPathSeparator();
                std::stringstream ss(pathEnv);
                std::string segment;
                while (std::getline(ss, segment, sep)) {
                    try {
                        for (const auto& entry : std::filesystem::directory_iterator(segment)) {
                            if (entry.is_regular_file()) {
                                std::string filename = entry.path().filename().string();
#ifdef _WIN32
                                if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".exe") {
                                    filename = filename.substr(0, filename.size() - 4);
                                }
#endif
                                if (filename.find(prefix) == 0) {
                                    matches.push_back(filename);
                                }
                            }
                        }
                    } catch (...) {}
                }
            }

            // 3. Files in current directory
            try {
                std::string dir = ".";
                std::string filePrefix = prefix;
                size_t lastSlash = prefix.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    dir = prefix.substr(0, lastSlash + 1);
                    filePrefix = prefix.substr(lastSlash + 1);
                }
                
                for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find(filePrefix) == 0) {
                        std::string fullMatch = (dir == "." ? "" : dir) + filename;
                        if (entry.is_directory()) fullMatch += "/";
                        matches.push_back(fullMatch);
                    }
                }
            } catch (...) {}

            return matches;
        };

        ShellState state;

        // Load .termidashrc
        std::string rcPath = PlatformUtils::getHomeDirectory() + "/.termidashrc";
        if (std::filesystem::exists(rcPath)) {
            runScript(rcPath, terminal, processManager);
        }

        while (true)
        {
            if (state.inBlock()) {
                terminal->write(">> ");
            } else {
                terminal->write("> ");
            }
            std::string input = readLineInteractive(terminal, history, history_index, completionGenerator);
            if (input.empty())
                continue;

            history.push_back(input);
            history_index = history.size();

            // Append to history file
            std::ofstream histOut(historyPath, std::ios::app);
            if (histOut.is_open()) {
                histOut << input << "\n";
            }

            processInputLine(input, builtInHandler, executor, processManager, jobManager.get(), state, nullptr, terminal);
        }
    }

    void runCommand(const std::string& commandLine, platform::ITerminal* terminal, platform::IProcessManager* processManager)
    {
        auto executorUP = createCommandExecutor();
        ICommandExecutor *executor = executorUP.get();
        auto jobManager = createJobManager();
        BuiltInCommandHandler builtInHandler;
        ShellState state;
        processInputLine(commandLine, builtInHandler, executor, processManager, jobManager.get(), state, nullptr, terminal);
    }

    void runScript(const std::string& path, platform::ITerminal* terminal, platform::IProcessManager* processManager)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "Failed to open script: " << path << "\n";
            return;
        }

        auto executorUP = createCommandExecutor();
        ICommandExecutor *executor = executorUP.get();
        auto jobManager = createJobManager();
        BuiltInCommandHandler builtInHandler;
        ShellState state;

        std::string line;
        while (std::getline(file, line))
        {
            std::string trimmed = trim(line);
            if (trimmed.empty() || trimmed[0] == '#') continue;
            processInputLine(line, builtInHandler, executor, processManager, jobManager.get(), state, &file, terminal);
        }
    }
}