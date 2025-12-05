#include "platform/windows/WindowsCommandExecutor.hpp"

#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> // for _strdup
#include <cstring> // for strlen, memcpy if needed

// Helper: tokenize command into args, respecting quotes
static std::vector<std::string> tokenizeCommand(const std::string &input)
{
    std::vector<std::string> tokens;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < input.size(); ++i)
    {
        char c = input[i];
        if (c == '\"')
        {
            inQuotes = !inQuotes;
            continue;
        }
        if (!inQuotes && (c == ' ' || c == '\t'))
        {
            if (!cur.empty())
            {
                tokens.push_back(cur);
                cur.clear();
            }
        }
        else
        {
            cur.push_back(c);
        }
    }
    if (!cur.empty())
        tokens.push_back(cur);
    return tokens;
}

int WindowsCommandExecutor::execute(const std::string &command, bool background)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    std::vector<std::string> tokens = tokenizeCommand(command);
    if (tokens.empty())
    {
        lastError = "Empty command";
        return -1;
    }

    // Rebuild command line
    std::string fullCommandLine;
    for (const auto &arg : tokens)
    {
        if (!fullCommandLine.empty())
            fullCommandLine += " ";
        fullCommandLine += arg;
    }

    char *cmdLine = _strdup(fullCommandLine.c_str());

    DWORD creationFlags = 0;
    if (background)
    {
        creationFlags |= CREATE_NEW_CONSOLE;
    }

    BOOL success = CreateProcessA(
        nullptr,
        cmdLine,
        nullptr,
        nullptr,
        FALSE,
        creationFlags,
        nullptr,
        nullptr,
        &si,
        &pi);

    free(cmdLine);

    if (!success)
    {
        DWORD errCode = GetLastError();
        lastError = std::string("CreateProcess failed with error code: ") + std::to_string(errCode);
        return -1;
    }

    if (!background)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

std::string WindowsCommandExecutor::getLastError()
{
    return lastError;
}
