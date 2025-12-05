#include "platform/windows/WindowsProcessManager.hpp"
#define NOMINMAX
#include <windows.h>
#include <iostream>

namespace termidash {
namespace platform {
namespace windows {

WindowsProcessManager::WindowsProcessManager() {}

long WindowsProcessManager::spawn(const std::string& command, const std::vector<std::string>& args, bool background,
                                  long stdIn, long stdOut, long stdErr) {
    std::string fullCommand = command;
    for (const auto& arg : args) {
        fullCommand += " " + arg;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = (stdIn != -1) ? (HANDLE)stdIn : GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = (stdOut != -1) ? (HANDLE)stdOut : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = (stdErr != -1) ? (HANDLE)stdErr : GetStdHandle(STD_ERROR_HANDLE);

    DWORD creationFlags = 0;
    if (background) {
        creationFlags |= CREATE_NEW_CONSOLE;
    }

    std::vector<char> cmdLine(fullCommand.begin(), fullCommand.end());
    cmdLine.push_back('\0');

    BOOL success = CreateProcessA(
        nullptr,
        cmdLine.data(),
        nullptr,
        nullptr,
        TRUE, // Inherit handles
        creationFlags,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!success) {
        lastError = "CreateProcess failed: " + std::to_string(GetLastError());
        return -1;
    }

    CloseHandle(pi.hThread);
    return (long)pi.hProcess;
}

bool WindowsProcessManager::createPipe(long& readHandle, long& writeHandle) {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) {
        lastError = "CreatePipe failed";
        return false;
    }
    readHandle = (long)hRead;
    writeHandle = (long)hWrite;
    return true;
}

void WindowsProcessManager::closeHandle(long handle) {
    if (handle != -1) {
        CloseHandle((HANDLE)handle);
    }
}

int WindowsProcessManager::wait(long pid) {
    HANDLE hProcess = (HANDLE)pid;
    DWORD result = WaitForSingleObject(hProcess, INFINITE);
    CloseHandle(hProcess); // Close it after waiting
    return (result == WAIT_OBJECT_0) ? 0 : -1;
}

bool WindowsProcessManager::kill(long pid) {
    HANDLE hProcess = (HANDLE)pid;
    bool result = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return result;
}

std::string WindowsProcessManager::getLastError() {
    return lastError;
}

} // namespace windows
} // namespace platform
} // namespace termidash
