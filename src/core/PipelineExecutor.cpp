#include "core/PipelineExecutor.hpp"
#include "core/Parser.hpp"
#include "core/InputHandler.hpp"
#include "core/ExecContext.hpp"
#include "core/MemStream.hpp"
#include "core/RingBuffer.hpp"
#include "common/PlatformUtils.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <memory>
#include <cstdlib>

namespace termidash {

std::string PipelineExecutor::readHereDoc(
    const std::string& delimiter,
    std::istream* inputSource,
    platform::ITerminal* terminal
) {
    std::string content;
    std::vector<std::string> dummyHist;
    size_t dummyIdx = 0;
    auto dummyGen = [](const std::string&) { return std::vector<std::string>{}; };
    
    while (true) {
        std::string line;
        if (inputSource) {
            if (!std::getline(*inputSource, line)) break;
        } else if (terminal) {
            terminal->write("> ");
            line = InputHandler::readLine(terminal, dummyHist, dummyIdx, dummyGen);
        } else {
            break;
        }
        if (Parser::trim(line) == delimiter) break;
        content += line + "\n";
    }
    return content;
}

int PipelineExecutor::executeSingle(
    const std::string& commandLine,
    BuiltInCommandHandler& builtInHandler,
    ICommandExecutor* executor,
    platform::IProcessManager* processManager,
    std::istream* inputSource,
    platform::ITerminal* terminal
) {
    std::string trimmed = Parser::trim(commandLine);
    if (trimmed.empty())
        return 0;

    auto redirInfo = Parser::parseRedirection(trimmed);
    std::string cleanCmd = redirInfo.command;
    std::string inFile = redirInfo.inFile;
    std::string outFile = redirInfo.outFile;
    std::string errFile = redirInfo.errFile;
    bool appendOut = redirInfo.appendOut;
    bool appendErr = redirInfo.appendErr;

    // Handle here-document
    if (redirInfo.isHereDoc) {
        std::string tempFile = ".heredoc_" + std::to_string(std::rand());
        std::ofstream tempOut(tempFile);
        if (!tempOut.is_open()) {
            std::cerr << "Error: Cannot create temporary file for here-document\n";
            return 1;
        }
        std::string content = readHereDoc(redirInfo.hereDocDelim, inputSource, terminal);
        tempOut << content;
        tempOut.close();
        inFile = tempFile;
    }

    // Get command name
    std::string cmdName;
    size_t spacePos = cleanCmd.find(' ');
    if (spacePos != std::string::npos)
        cmdName = cleanCmd.substr(0, spacePos);
    else
        cmdName = cleanCmd;

    // Check if it's a built-in command
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
        return builtInHandler.handleCommandWithContext(cleanCmd, ctx);
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
    auto tokens = Parser::tokenize(cleanCmd);
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

    if (pid == -1) {
        std::cerr << "Error: Failed to spawn: " << cmd << " Error: " << processManager->getLastError() << "\n";
        return 1;
    }

    return processManager->wait(pid);
}

int PipelineExecutor::executeBuiltInPipeline(
    const std::vector<SegmentInfo>& segments,
    BuiltInCommandHandler& builtInHandler
) {
    size_t n = segments.size();
    std::vector<std::shared_ptr<termidash::StreamBridge>> bridges;
    for (size_t i = 0; i < n - 1; ++i) {
        bridges.push_back(std::make_shared<termidash::StreamBridge>());
    }

    std::vector<std::thread> threads;
    threads.reserve(n);
    std::vector<int> exitCodes(n, 0);

    for (size_t i = 0; i < n; ++i) {
        SegmentInfo info = segments[i];
        std::shared_ptr<termidash::StreamBridge> prevBridge = (i > 0) ? bridges[i - 1] : nullptr;
        std::shared_ptr<termidash::StreamBridge> nextBridge = (i < n - 1) ? bridges[i] : nullptr;

        std::thread th([info, prevBridge, nextBridge, &builtInHandler, &exitCodes, i]() {
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

int PipelineExecutor::executeExternalPipeline(
    const std::vector<SegmentInfo>& segments,
    BuiltInCommandHandler& builtInHandler,
    platform::IProcessManager* processManager
) {
    size_t n = segments.size();
    std::vector<long> pids;
    long prevRead = -1;

    for (size_t i = 0; i < n; ++i) {
        long nextRead = -1;
        long nextWrite = -1;

        if (i < n - 1) {
            if (!processManager->createPipe(nextRead, nextWrite)) {
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
        }

        if (!segments[i].outFile.empty()) {
            stdOut = PlatformUtils::openFileForWrite(segments[i].outFile, segments[i].appendOut);
            if (stdOut == -1) std::cerr << "Error: Cannot open output file: " << segments[i].outFile << "\n";
        } else if (i < n - 1) {
            stdOut = nextWrite;
        }

        if (!segments[i].errFile.empty()) {
            if (segments[i].errFile == segments[i].outFile && stdOut != -1) {
                stdErr = stdOut;
            } else {
                stdErr = PlatformUtils::openFileForWrite(segments[i].errFile, segments[i].appendErr);
                if (stdErr == -1) std::cerr << "Error: Cannot open error file: " << segments[i].errFile << "\n";
            }
        }

        auto tokens = Parser::tokenize(segments[i].cleanCmd);
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

        if (pid == -1) {
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
    for (long pid : pids) {
        int code = processManager->wait(pid);
        if (pid == pids.back()) lastExitCode = code;
    }

    return lastExitCode;
}

int PipelineExecutor::execute(
    const std::string& pipelineLine,
    BuiltInCommandHandler& builtInHandler,
    ICommandExecutor* executor,
    platform::IProcessManager* processManager,
    std::istream* inputSource,
    platform::ITerminal* terminal
) {
    auto rawSegments = Parser::splitPipelineOperators(pipelineLine);
    if (rawSegments.empty())
        return 0;

    std::vector<SegmentInfo> segments;
    bool allBuiltIn = true;

    for (const auto& raw : rawSegments) {
        SegmentInfo info;
        info.trimBeforeNext = raw.trimBeforeNext;
        
        auto redirInfo = Parser::parseRedirection(raw.cmd);
        info.cleanCmd = redirInfo.command;
        info.inFile = redirInfo.inFile;
        info.outFile = redirInfo.outFile;
        info.errFile = redirInfo.errFile;
        info.appendOut = redirInfo.appendOut;
        info.appendErr = redirInfo.appendErr;
        info.hereDocDelim = redirInfo.hereDocDelim;
        info.isHereDoc = redirInfo.isHereDoc;

        // Handle here-document
        if (info.isHereDoc) {
            std::string tempFile = ".heredoc_" + std::to_string(std::rand());
            std::ofstream tempOut(tempFile);
            if (tempOut.is_open()) {
                std::string content = readHereDoc(info.hereDocDelim, inputSource, terminal);
                tempOut << content;
                tempOut.close();
                info.inFile = tempFile;
            } else {
                std::cerr << "Error: Cannot create temporary file for here-document\n";
            }
        }

        segments.push_back(info);

        // Check if command is built-in
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

    if (allBuiltIn) {
        return executeBuiltInPipeline(segments, builtInHandler);
    } else {
        return executeExternalPipeline(segments, builtInHandler, processManager);
    }
}

} // namespace termidash
