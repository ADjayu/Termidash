#pragma once

#include "core/ISignalHandler.hpp"
#include <windows.h>

namespace termidash {
namespace platform {
namespace windows {

class WindowsSignalHandler : public ISignalHandler {
public:
    void setupHandlers() override;
    void resetHandlers() override;

private:
    static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);
};

} // namespace windows
} // namespace platform
} // namespace termidash
