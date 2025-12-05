#pragma once
#include <string>
#include <map>
#include <vector>

namespace termidash {

class VariableManager {
public:
    static VariableManager& instance();

    void set(const std::string& name, const std::string& value);
    std::string get(const std::string& name) const;
    bool has(const std::string& name) const;
    void unset(const std::string& name);
    std::map<std::string, std::string> getAll() const;

    void pushScope();
    void popScope();

private:
    VariableManager() = default;
    ~VariableManager() = default;
    VariableManager(const VariableManager&) = delete;
    VariableManager& operator=(const VariableManager&) = delete;

    std::map<std::string, std::string> variables;
    std::vector<std::map<std::string, std::string>> scopes;
};

} // namespace termidash
