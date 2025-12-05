#pragma once
#include <string>

namespace termidash {
namespace platform {

/**
 * @brief Error types for process operations
 */
enum class ProcessError {
    None = 0,           // No error
    SpawnFailed,        // Failed to spawn/fork process
    PipeFailed,         // Failed to create pipe
    WaitFailed,         // Failed to wait for process
    KillFailed,         // Failed to kill process
    PermissionDenied,   // Permission denied
    NotFound,           // Executable not found
    InvalidArgument,    // Invalid argument provided
    ResourceLimit,      // Resource limit exceeded (e.g., too many processes)
    Timeout,            // Operation timed out
    Unknown             // Unknown error
};

/**
 * @brief Result from process operations
 * 
 * Provides structured error information instead of just -1 on failure.
 */
struct ProcessResult {
    long pid = -1;              // Process ID, -1 on failure
    int exitCode = -1;          // Exit code from wait(), -1 if not waited
    ProcessError error = ProcessError::None;
    std::string errorMessage;   // Human-readable error message
    
    /**
     * @brief Check if operation succeeded
     */
    bool success() const { return error == ProcessError::None && pid != -1; }
    
    /**
     * @brief Create a success result
     */
    static ProcessResult ok(long pid) {
        ProcessResult r;
        r.pid = pid;
        r.error = ProcessError::None;
        return r;
    }
    
    /**
     * @brief Create an error result
     */
    static ProcessResult fail(ProcessError err, const std::string& msg = "") {
        ProcessResult r;
        r.pid = -1;
        r.error = err;
        r.errorMessage = msg;
        return r;
    }
};

/**
 * @brief Result from pipe creation
 */
struct PipeResult {
    long readHandle = -1;
    long writeHandle = -1;
    ProcessError error = ProcessError::None;
    std::string errorMessage;
    
    bool success() const { return error == ProcessError::None; }
    
    static PipeResult ok(long read, long write) {
        PipeResult r;
        r.readHandle = read;
        r.writeHandle = write;
        r.error = ProcessError::None;
        return r;
    }
    
    static PipeResult fail(ProcessError err, const std::string& msg = "") {
        PipeResult r;
        r.error = err;
        r.errorMessage = msg;
        return r;
    }
};

/**
 * @brief Convert ProcessError to string for logging
 */
inline std::string toString(ProcessError err) {
    switch (err) {
        case ProcessError::None: return "None";
        case ProcessError::SpawnFailed: return "SpawnFailed";
        case ProcessError::PipeFailed: return "PipeFailed";
        case ProcessError::WaitFailed: return "WaitFailed";
        case ProcessError::KillFailed: return "KillFailed";
        case ProcessError::PermissionDenied: return "PermissionDenied";
        case ProcessError::NotFound: return "NotFound";
        case ProcessError::InvalidArgument: return "InvalidArgument";
        case ProcessError::ResourceLimit: return "ResourceLimit";
        case ProcessError::Timeout: return "Timeout";
        case ProcessError::Unknown: return "Unknown";
        default: return "Unknown";
    }
}

} // namespace platform
} // namespace termidash
