#include "core/BuiltIn/WindowsCommandHandler.hpp"
#include "core/ExecContext.hpp"
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <unordered_set>

namespace termidash {

static bool startsWithDigit(const std::string &s)
{
    return !s.empty() && isdigit(static_cast<unsigned char>(s[0]));
}

bool WindowsCommandHandler::handle(const std::vector<std::string> &tokens)
{
    ExecContext ctx(std::cin, std::cout, std::cerr);
    return handleWithContext(tokens, ctx);
}

int WindowsCommandHandler::handleWithContext(const std::vector<std::string> &tokens, ExecContext &ctx)
{
    if (tokens.empty())
        return -1;

    const std::string &cmd = tokens[0];

    if (cmd == "cd")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "cd: missing operand\n";
            return 1;
        }
        else if (!SetCurrentDirectoryA(tokens[1].c_str()))
        {
            ctx.err << "cd: failed to change directory\n";
            return 1;
        }
        return 0;
    }
    else if (cmd == "cls")
    {
        system("cls");
        return 0;
    }
    else if (cmd == "ver")
    {
        OSVERSIONINFO versionInfo;
        ZeroMemory(&versionInfo, sizeof(OSVERSIONINFO));
        versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionExA(&versionInfo))
        {
            ctx.out << "Windows Version: " << versionInfo.dwMajorVersion << "."
                    << versionInfo.dwMinorVersion << " (Build " << versionInfo.dwBuildNumber << ")\n";
            return 0;
        }
        else
        {
            ctx.err << "ver: failed to get version info\n";
            return 1;
        }
    }
    else if (cmd == "getenv")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "getenv: missing variable name\n";
            return 1;
        }
        else
        {
            char buffer[32767];
            DWORD len = GetEnvironmentVariableA(tokens[1].c_str(), buffer, sizeof(buffer));
            if (len > 0)
            {
                ctx.out << tokens[1] << "=" << buffer << "\n";
                return 0;
            }
            else
            {
                ctx.err << "getenv: variable not found\n";
                return 1;
            }
        }
    }
    else if (cmd == "setenv")
    {
        if (tokens.size() < 3)
        {
            ctx.err << "setenv: missing arguments. Usage: setenv VAR VALUE\n";
            return 1;
        }
        else
        {
            if (!SetEnvironmentVariableA(tokens[1].c_str(), tokens[2].c_str()))
            {
                ctx.err << "setenv: failed to set environment variable\n";
                return 1;
            }
            return 0;
        }
    }
    else if (cmd == "cwd")
    {
        char buffer[MAX_PATH];
        if (GetCurrentDirectoryA(MAX_PATH, buffer))
        {
            ctx.out << buffer << std::endl;
            return 0;
        }
        else
        {
            ctx.err << "cwd: failed to get current directory\n";
            return 1;
        }
    }
    else if (cmd == "drives")
    {
        DWORD drives = GetLogicalDrives();
        if (drives == 0)
        {
            ctx.err << "drives: failed to get drives\n";
            return 1;
        }
        else
        {
            ctx.out << "Available drives: ";
            for (char letter = 'A'; letter <= 'Z'; ++letter)
            {
                if (drives & (1 << (letter - 'A')))
                {
                    ctx.out << letter << ": ";
                }
            }
            ctx.out << "\n";
            return 0;
        }
    }
    else if (cmd == "type")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "type: missing file operand\n";
            return 1;
        }
        else
        {
            FILE *file = fopen(tokens[1].c_str(), "r");
            if (!file)
            {
                ctx.err << "type: cannot open file\n";
                return 1;
            }
            else
            {
                char ch;
                while ((ch = fgetc(file)) != EOF)
                {
                    ctx.out << ch;
                }
                fclose(file);
                ctx.out << std::endl;
                return 0;
            }
        }
    }
    else if (cmd == "mkdir")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "mkdir: missing directory operand\n";
            return 1;
        }
        else
        {
            if (!CreateDirectoryA(tokens[1].c_str(), NULL))
            {
                ctx.err << "mkdir: failed to create directory\n";
                return 1;
            }
            return 0;
        }
    }
    else if (cmd == "rmdir")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "rmdir: missing directory operand\n";
            return 1;
        }
        else
        {
            if (!RemoveDirectoryA(tokens[1].c_str()))
            {
                ctx.err << "rmdir: failed to remove directory (must be empty)\n";
                return 1;
            }
            return 0;
        }
    }
    else if (cmd == "copy")
    {
        if (tokens.size() < 3)
        {
            ctx.err << "copy: missing source or destination\n";
            return 1;
        }
        else
        {
            if (!CopyFileA(tokens[1].c_str(), tokens[2].c_str(), FALSE))
            {
                ctx.err << "copy: failed to copy file\n";
                return 1;
            }
            return 0;
        }
    }
    else if (cmd == "del")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "del: missing file operand\n";
            return 1;
        }
        else
        {
            if (!DeleteFileA(tokens[1].c_str()))
            {
                ctx.err << "del: failed to delete file\n";
                return 1;
            }
            return 0;
        }
    }
    else if (cmd == "tasklist")
    {
        return system("tasklist");
    }
    else if (cmd == "taskkill")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "taskkill: missing PID or task name\n";
            return 1;
        }
        else
        {
            std::string arg = tokens[1];
            std::string command = "taskkill /F ";
            if (startsWithDigit(arg))
            {
                command += "/PID " + arg;
            }
            else
            {
                command += "/IM " + arg;
            }
            return system(command.c_str());
        }
    }
    else if (cmd == "ping")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "ping: missing address\n";
            return 1;
        }
        else
        {
            std::string command = "ping " + tokens[1];
            return system(command.c_str());
        }
    }
    else if (cmd == "ipconfig")
    {
        return system("ipconfig");
    }
    else if (cmd == "whoami")
    {
        return system("whoami");
    }
    else if (cmd == "hostname")
    {
        return system("hostname");
    }
    else if (cmd == "assoc")
    {
        return system("assoc");
    }
    else if (cmd == "systeminfo")
    {
        return system("systeminfo");
    }
    else if (cmd == "netstat")
    {
        return system("netstat -an");
    }
    else if (cmd == "echo")
    {
        for (size_t i = 1; i < tokens.size(); ++i)
        {
            ctx.out << tokens[i] << " ";
        }
        ctx.out << "\n";
        return 0;
    }
    else if (cmd == "pause")
    {
        return system("pause");
    }
    else if (cmd == "time")
    {
        return system("time /t");
    }
    else if (cmd == "date")
    {
        return system("date /t");
    }
    else if (cmd == "dir")
    {
        return system("dir");
    }
    else if (cmd == "attrib")
    {
        if (tokens.size() < 2)
        {
            ctx.err << "attrib: missing file operand\n";
            return 1;
        }
        else
        {
            std::string command = "attrib " + tokens[1];
            return system(command.c_str());
        }
    }

    return -1; // Unhandled command
}

bool WindowsCommandHandler::isCommand(const std::string &cmd) const
{
    static const std::unordered_set<std::string> cmds = {
        "assoc", "attrib", "break", "bcdedit", "cacls", "cd", "chcp", "chdir", "chkdsk", "chkntfs",
        "cls", "color", "comp", "compact", "convert", "copy", "date", "del", "dir", "diskpart",
        "doskey", "driverquery", "echo", "endlocal", "erase", "exit", "fc",
        "for", "format", "fsutil", "ftype", "goto", "gpresult", "graftabl", "help", "icacls", "if",
        "label", "md", "mkdir", "mklink", "mode", "move", "openfiles", "path", "pause",
        "popd", "print", "prompt", "pushd", "rd", "recover", "rem", "rename", "replace", "rmdir",
        "robocopy", "set", "setlocal", "sc", "schtasks", "shift", "shutdown", "start", "subst",
        "systeminfo", "tasklist", "taskkill", "time", "title", "tree", "type", "ver", "verify", "vol",
        "where", "whoami", "xcopy"};
    return cmds.find(cmd) != cmds.end();
}

} // namespace termidash
