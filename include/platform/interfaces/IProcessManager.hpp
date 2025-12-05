#pragma once
#include <string>
#include <vector>

namespace termidash {
namespace platform {

class IProcessManager {
public:
    virtual ~IProcessManager() = default;

    // Process execution
    // Returns process ID or handle as an integer/long, or -1 on failure
    // stdIn, stdOut, stdErr: handles/file descriptors. -1 means default (inherit/console).
    virtual long spawn(const std::string& command, const std::vector<std::string>& args, bool background, 
                       long stdIn = -1, long stdOut = -1, long stdErr = -1) = 0;
    
    // Wait for a specific process
    virtual int wait(long pid) = 0;
    
    // Kill a specific process
    virtual bool kill(long pid) = 0;

    // Pipe management
    // Creates a pipe. Returns true on success.
    // readHandle and writeHandle are output parameters.
    virtual bool createPipe(long& readHandle, long& writeHandle) = 0;

    // Close a handle/file descriptor
    virtual void closeHandle(long handle) = 0;

    // Get last error message
    virtual std::string getLastError() = 0;
};

} // namespace platform
} // namespace termidash
