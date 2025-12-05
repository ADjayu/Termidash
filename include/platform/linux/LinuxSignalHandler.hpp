#pragma once

#include "core/ISignalHandler.hpp"
#include <signal.h>

namespace termidash {
namespace platform {
namespace linux_platform {

class LinuxSignalHandler : public ISignalHandler {
public:
    void setupHandlers() override;
    void resetHandlers() override;

private:
    static void handleSignal(int signal);
};

} // namespace linux_platform
} // namespace platform
} // namespace termidash
