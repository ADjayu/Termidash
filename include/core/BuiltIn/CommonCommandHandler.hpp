#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "core/ExecContext.hpp"
#include "core/AliasManager.hpp"
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

namespace termidash {

class CommonCommandHandler {
public:
    bool isCommand(const std::string& cmd) const;
public:
    CommonCommandHandler();
    bool handle(const std::string& input, const std::vector<std::string>& tokens);
    int handleWithContext(const std::string& input, const std::vector<std::string>& tokens, ::ExecContext& ctx);
    void handleHistory(::ExecContext& ctx) const;
    void loadHistory(const std::string& path);
    void saveHistory(const std::string& path) const;
    std::vector<std::string> tokenize(const std::string& input) const;
    const std::vector<std::string>& getHistory() const;

private:
    std::vector<std::string> history;
};

} // namespace termidash
