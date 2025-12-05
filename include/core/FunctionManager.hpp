#pragma once
#include <string>
#include <map>
#include <vector>

namespace termidash {

class FunctionManager {
public:
    static FunctionManager& instance();

    void define(const std::string& name, const std::vector<std::string>& body);
    bool has(const std::string& name) const;
    const std::vector<std::string>& getBody(const std::string& name) const;
    void unset(const std::string& name);
    std::map<std::string, std::vector<std::string>> getAll() const;

private:
    FunctionManager() = default;
    ~FunctionManager() = default;
    FunctionManager(const FunctionManager&) = delete;
    FunctionManager& operator=(const FunctionManager&) = delete;

    std::map<std::string, std::vector<std::string>> functions;
};

} // namespace termidash
