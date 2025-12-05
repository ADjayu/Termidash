#pragma once
#include "platform/interfaces/IProcessManager.hpp"
#include <sys/types.h>

namespace termidash {
namespace platform {
namespace linux_platform {

class LinuxProcessManager : public IProcessManager {
public:
    LinuxProcessManager() = default;
    ~LinuxProcessManager() override = default;

    long spawn(const std::string& command, const std::vector<std::string>& args, bool background,
               long stdIn = -1, long stdOut = -1, long stdErr = -1) override;
    int wait(long pid) override;
    bool kill(long pid) override;
    bool createPipe(long& readHandle, long& writeHandle) override;
    void closeHandle(long handle) override;
    std::string getLastError() override;

private:
    std::string lastError;
};

} // namespace linux_platform
} // namespace platform
} // namespace termidash
