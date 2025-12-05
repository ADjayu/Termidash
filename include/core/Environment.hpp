#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace termidash {

class Environment {
public:
    static Environment& instance();

    void set(const std::string& key, const std::string& value, bool exportVar = false);
    std::string get(const std::string& key) const;
    void unset(const std::string& key);
    
    std::unordered_map<std::string, std::string> getExported() const;
    std::unordered_map<std::string, std::string> getAll() const;

    std::string expand(const std::string& input) const;

private:
    Environment() = default;
    
    struct Variable {
        std::string value;
        bool isExported;
    };

    std::unordered_map<std::string, Variable> variables;
};

} // namespace termidash
