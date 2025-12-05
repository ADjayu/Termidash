#pragma once
#include <string>
#include <vector>
#include "platform/interfaces/ProcessError.hpp"

namespace termidash {
namespace platform {

/**
 * @brief Interface for platform-specific process management
 * 
 * Provides methods for spawning, waiting, and killing processes,
 * as well as pipe management for inter-process communication.
 */
class IProcessManager {
public:
    virtual ~IProcessManager() = default;

    // =========================================================================
    // Legacy API (for backward compatibility)
    // =========================================================================
    
    /**
     * @brief Spawn a process
     * @param command Executable name or path
     * @param args Command arguments
     * @param background If true, spawn in background
     * @param stdIn Input handle (-1 for default)
     * @param stdOut Output handle (-1 for default)
     * @param stdErr Error handle (-1 for default)
     * @return Process ID/handle, or -1 on failure
     */
    virtual long spawn(const std::string& command, const std::vector<std::string>& args, bool background, 
                       long stdIn = -1, long stdOut = -1, long stdErr = -1) = 0;
    
    /**
     * @brief Wait for a specific process to complete
     * @param pid Process ID/handle
     * @return Exit code, or -1 on failure
     */
    virtual int wait(long pid) = 0;
    
    /**
     * @brief Kill a specific process
     * @param pid Process ID/handle
     * @return true if successful
     */
    virtual bool kill(long pid) = 0;

    /**
     * @brief Create a pipe
     * @param readHandle Output: read end of pipe
     * @param writeHandle Output: write end of pipe
     * @return true if successful
     */
    virtual bool createPipe(long& readHandle, long& writeHandle) = 0;

    /**
     * @brief Close a handle/file descriptor
     */
    virtual void closeHandle(long handle) = 0;

    /**
     * @brief Get last error message
     */
    virtual std::string getLastError() = 0;

    // =========================================================================
    // New API with structured error handling
    // =========================================================================

    /**
     * @brief Spawn a process with structured result
     * @return ProcessResult with pid, error info
     */
    virtual ProcessResult spawnProcess(const std::string& command, 
                                       const std::vector<std::string>& args, 
                                       bool background,
                                       long stdIn = -1, long stdOut = -1, long stdErr = -1) {
        // Default implementation wraps legacy spawn()
        long pid = spawn(command, args, background, stdIn, stdOut, stdErr);
        if (pid == -1) {
            return ProcessResult::fail(ProcessError::SpawnFailed, getLastError());
        }
        return ProcessResult::ok(pid);
    }

    /**
     * @brief Wait for process with structured result
     * @return ProcessResult with exit code
     */
    virtual ProcessResult waitProcess(long pid) {
        // Default implementation wraps legacy wait()
        int exitCode = wait(pid);
        ProcessResult result;
        result.pid = pid;
        result.exitCode = exitCode;
        if (exitCode == -1) {
            result.error = ProcessError::WaitFailed;
            result.errorMessage = getLastError();
        }
        return result;
    }

    /**
     * @brief Kill process with structured result
     * @return ProcessResult indicating success/failure
     */
    virtual ProcessResult killProcess(long pid) {
        // Default implementation wraps legacy kill()
        bool success = kill(pid);
        if (!success) {
            return ProcessResult::fail(ProcessError::KillFailed, getLastError());
        }
        ProcessResult result;
        result.pid = pid;
        return result;
    }

    /**
     * @brief Create a pipe with structured result
     * @return PipeResult with handles or error
     */
    virtual PipeResult createPipeEx() {
        // Default implementation wraps legacy createPipe()
        long read = -1, write = -1;
        if (!createPipe(read, write)) {
            return PipeResult::fail(ProcessError::PipeFailed, getLastError());
        }
        return PipeResult::ok(read, write);
    }
};

} // namespace platform
} // namespace termidash
