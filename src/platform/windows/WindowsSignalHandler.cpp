#include "platform/windows/WindowsSignalHandler.hpp"
#include <iostream>

namespace termidash {
namespace platform {
namespace windows {

void WindowsSignalHandler::setupHandlers() {
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        std::cerr << "Error: Could not set control handler" << std::endl;
    }
}

void WindowsSignalHandler::resetHandlers() {
    SetConsoleCtrlHandler(CtrlHandler, FALSE);
}

BOOL WINAPI WindowsSignalHandler::CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        // Handle the CTRL-C signal.
        // For now, we just print a newline and prompt again (simulated by ignoring default exit)
        // In a real shell, we would check if a child process is running.
        std::cout << "\n^C" << std::endl;
        // Return TRUE to indicate signal is handled and prevent default termination
        return TRUE;

    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        return FALSE;

    default:
        return FALSE;
    }
}

} // namespace windows
} // namespace platform
} // namespace termidash
