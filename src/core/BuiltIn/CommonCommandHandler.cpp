#include "core/BuiltIn/CommonCommandHandler.hpp"
#include "core/AliasManager.hpp"
#include "core/VariableManager.hpp"
#include "core/PromptEngine.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

namespace termidash
{
    CommonCommandHandler::CommonCommandHandler()
    {
    }

    void CommonCommandHandler::loadHistory(const std::string& path)
    {
        history.clear();
        std::ifstream file(path);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                if (!line.empty())
                    history.push_back(line);
            }
        }
    }

    void CommonCommandHandler::saveHistory(const std::string& path) const
    {
        std::ofstream file(path);
        if (file.is_open())
        {
            for (const auto& line : history)
            {
                file << line << "\n";
            }
        }
    }

    int CommonCommandHandler::handleWithContext(const std::string &input, const std::vector<std::string> &tokens, ::ExecContext &ctx)
    {
        if (tokens.empty())
            return -1;

        const std::string &cmd = tokens[0];

        if (cmd == "help")
        {
            ctx.out << "Available commands:\n";
            ctx.out << "  cd, cls, ver, getenv, setenv, cwd, drives, type, mkdir, rmdir, copy, del\n";
            ctx.out << "  tasklist, taskkill, ping, ipconfig, whoami, hostname, assoc, systeminfo, netstat\n";
            ctx.out << "  echo, pause, time, date, dir, attrib\n";
            ctx.out << "  clear, help, exit, version, alias, unalias, pwd, touch, rm, cat, uptime, grep, sort, head, tail, history\n";
            return 0;
        }
        else if (cmd == "clear")
        {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            return 0;
        }
        else if (cmd == "version")
        {
            ctx.out << "Termidash Shell Version 1.0.0\n";
            return 0;
        }
        else if (cmd == "exit")
        {
            exit(0);
        }
        else if (cmd == "alias")
        {
            if (tokens.size() == 1)
            {
                auto aliases = AliasManager::instance().getAll();
                for (const auto &pair : aliases)
                {
                    ctx.out << pair.first << "='" << pair.second << "'\n";
                }
            }
            else
            {
                std::string args;
                for (size_t i = 1; i < tokens.size(); ++i)
                {
                    if (i > 1)
                        args += " ";
                    args += tokens[i];
                }

                size_t eqPos = args.find('=');
                if (eqPos != std::string::npos)
                {
                    std::string name = args.substr(0, eqPos);
                    std::string value = args.substr(eqPos + 1);
                    if (value.size() >= 2 && (value.front() == '\'' || value.front() == '"') && value.back() == value.front())
                    {
                        value = value.substr(1, value.size() - 2);
                    }
                    AliasManager::instance().set(name, value);
                }
                else
                {
                    if (AliasManager::instance().has(args))
                    {
                        ctx.out << args << "='" << AliasManager::instance().get(args) << "'\n";
                    }
                    else
                    {
                        ctx.err << "alias: " << args << ": not found\n";
                    }
                }
            }
            return 0;
        }
        else if (cmd == "unalias")
        {
            if (tokens.size() < 2)
            {
                ctx.err << "unalias: usage: unalias name [name ...]\n";
                return 1;
            }
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                if (tokens[i] == "-a") continue;
                AliasManager::instance().unset(tokens[i]);
            }
            return 0;
        }
        else if (cmd == "unset")
        {
            if (tokens.size() < 2)
            {
                ctx.err << "unset: usage: unset name [name ...]\n";
                return 1;
            }
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                VariableManager::instance().unset(tokens[i]);
            }
            return 0;
        }
        else if (cmd == "export")
        {
            if (tokens.size() < 2)
            {
                // List all exported variables
                auto vars = VariableManager::instance().getAll();
                for (const auto& pair : vars)
                {
                    ctx.out << "export " << pair.first << "=\"" << pair.second << "\"\n";
                }
                return 0;
            }
            // Parse VAR=value or VAR="value" or VAR='value'
            std::string args;
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                if (i > 1) args += " ";
                args += tokens[i];
            }
            
            size_t eqPos = args.find('=');
            if (eqPos != std::string::npos)
            {
                std::string varName = args.substr(0, eqPos);
                std::string value = args.substr(eqPos + 1);
                // Remove quotes if present
                if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                    (value.front() == '\'' && value.back() == '\'')))
                {
                    value = value.substr(1, value.size() - 2);
                }
                
                // Handle PS1 specially
                if (varName == "PS1")
                {
                    PromptEngine::instance().setPS1(value);
                }
                
                VariableManager::instance().set(varName, value);
            }
            return 0;
        }
        else if (cmd == "set")
        {
            // List all variables
            auto vars = VariableManager::instance().getAll();
            for (const auto& pair : vars)
            {
                ctx.out << pair.first << "=" << pair.second << "\n";
            }
            return 0;
        }
        else if (cmd == "pwd")
        {
            char buffer[1024];
            if (getcwd(buffer, sizeof(buffer)) != NULL)
            {
                ctx.out << buffer << "\n";
                return 0;
            }
            else
            {
                perror("getcwd() error");
                return 1;
            }
        }
        else if (cmd == "touch")
        {
            if (tokens.size() < 2)
            {
                ctx.err << "touch: missing file operand\n";
                return 1;
            }
            int ret = 0;
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                std::ofstream file(tokens[i], std::ios::app);
                if (!file)
                {
                    ctx.err << "touch: cannot touch '" << tokens[i] << "'\n";
                    ret = 1;
                }
            }
            return ret;
        }
        else if (cmd == "rm")
        {
            if (tokens.size() < 2)
            {
                ctx.err << "rm: missing operand\n";
                return 1;
            }
            int ret = 0;
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                if (remove(tokens[i].c_str()) != 0)
                {
                    ctx.err << "rm: cannot remove '" << tokens[i] << "'\n";
                    ret = 1;
                }
            }
            return ret;
        }
        else if (cmd == "cat")
        {
            if (tokens.size() < 2)
            {
                // Read from stdin
                std::string line;
                while (std::getline(ctx.in, line)) {
                    ctx.out << line << "\n";
                }
                return 0;
            }
            int ret = 0;
            for (size_t i = 1; i < tokens.size(); ++i)
            {
                std::ifstream file(tokens[i]);
                if (file)
                {
                    ctx.out << file.rdbuf() << "\n";
                }
                else
                {
                    ctx.err << "cat: " << tokens[i] << ": No such file or directory\n";
                    ret = 1;
                }
            }
            return ret;
        }
        else if (cmd == "uptime")
        {
            ctx.out << "uptime: not implemented\n";
            return 0;
        }
        else if (cmd == "history")
        {
            for (size_t i = 0; i < history.size(); ++i)
            {
                ctx.out << i + 1 << "  " << history[i] << "\n";
            }
            return 0;
        }
        else if (cmd == "grep")
        {
             if (tokens.size() < 3) {
                ctx.err << "grep: usage: grep pattern file\n";
                return 1;
            }
            std::string pattern = tokens[1];
            std::string filename = tokens[2];
            std::ifstream file(filename);
            if (!file) {
                ctx.err << "grep: " << filename << ": No such file\n";
                return 1;
            }
            std::string line;
            bool found = false;
            while (std::getline(file, line)) {
                if (line.find(pattern) != std::string::npos) {
                    ctx.out << line << "\n";
                    found = true;
                }
            }
            return found ? 0 : 1;
        }
        else if (cmd == "sort")
        {
            if (tokens.size() < 2) {
                ctx.err << "sort: usage: sort file\n";
                return 1;
            }
            std::string filename = tokens[1];
            std::ifstream file(filename);
             if (!file) {
                ctx.err << "sort: " << filename << ": No such file\n";
                return 1;
            }
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
            std::sort(lines.begin(), lines.end());
            for (const auto& l : lines) {
                ctx.out << l << "\n";
            }
            return 0;
        }
        else if (cmd == "head")
        {
             if (tokens.size() < 2) {
                ctx.err << "head: usage: head file\n";
                return 1;
            }
            std::string filename = tokens[1];
            std::ifstream file(filename);
             if (!file) {
                ctx.err << "head: " << filename << ": No such file\n";
                return 1;
            }
            std::string line;
            int count = 0;
            while (count < 10 && std::getline(file, line)) {
                ctx.out << line << "\n";
                count++;
            }
            return 0;
        }
        else if (cmd == "tail")
        {
             if (tokens.size() < 2) {
                ctx.err << "tail: usage: tail file\n";
                return 1;
            }
            std::string filename = tokens[1];
            std::ifstream file(filename);
             if (!file) {
                ctx.err << "tail: " << filename << ": No such file\n";
                return 1;
            }
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }
            int n = 10;
            int start = (std::max)(0, (int)lines.size() - n);
            for (int i = start; i < lines.size(); ++i) {
                ctx.out << lines[i] << "\n";
            }
            return 0;
        }


        return -1;
    }

    bool CommonCommandHandler::isCommand(const std::string& cmd) const
    {
        static const std::vector<std::string> commands = {
            "cd", "cls", "ver", "getenv", "setenv", "cwd", "drives", "type", "mkdir", "rmdir", "copy", "del",
            "tasklist", "taskkill", "ping", "ipconfig", "whoami", "hostname", "assoc", "systeminfo", "netstat",
            "echo", "pause", "time", "date", "dir", "attrib", "help", "clear", "exit", "version", "alias", "unalias",
            "pwd", "touch", "rm", "cat", "uptime", "history", "grep", "sort", "head", "tail", "unset", "export", "set"
        };
        return std::find(commands.begin(), commands.end(), cmd) != commands.end();
    }

    bool CommonCommandHandler::handle(const std::string& input, const std::vector<std::string>& tokens)
    {
        ExecContext ctx(std::cin, std::cout, std::cerr);
        return handleWithContext(input, tokens, ctx) == 0;
    }

    void CommonCommandHandler::handleHistory(::ExecContext& ctx) const
    {
        for (size_t i = 0; i < history.size(); ++i)
        {
            ctx.out << i + 1 << "  " << history[i] << "\n";
        }
    }

    std::vector<std::string> CommonCommandHandler::tokenize(const std::string& input) const
    {
        std::stringstream ss(input);
        std::string segment;
        std::vector<std::string> tokens;
        while (std::getline(ss, segment, ' '))
        {
            if (!segment.empty())
                tokens.push_back(segment);
        }
        return tokens;
    }

    const std::vector<std::string>& CommonCommandHandler::getHistory() const
    {
        return history;
    }
}
