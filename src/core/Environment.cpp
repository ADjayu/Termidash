#include "core/Environment.hpp"
#include <sstream>

namespace termidash {

Environment& Environment::instance() {
    static Environment env;
    return env;
}

void Environment::set(const std::string& key, const std::string& value, bool exportVar) {
    variables[key] = {value, exportVar};
}

std::string Environment::get(const std::string& key) const {
    auto it = variables.find(key);
    if (it != variables.end()) {
        return it->second.value;
    }
    // Fallback to system environment
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : "";
}

void Environment::unset(const std::string& key) {
    variables.erase(key);
}

std::unordered_map<std::string, std::string> Environment::getExported() const {
    std::unordered_map<std::string, std::string> exported;
    for (const auto& [key, var] : variables) {
        if (var.isExported) {
            exported[key] = var.value;
        }
    }
    return exported;
}

std::unordered_map<std::string, std::string> Environment::getAll() const {
    std::unordered_map<std::string, std::string> all;
    for (const auto& [key, var] : variables) {
        all[key] = var.value;
    }
    return all;
}

std::string Environment::expand(const std::string& input) const {
    std::string result;
    std::string varName;
    bool inVar = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (c == '$') {
            if (inVar) {
                result += get(varName);
                varName.clear();
            }
            inVar = true;
        } else if (inVar) {
            if (isalnum(c) || c == '_') {
                varName += c;
            } else {
                result += get(varName);
                varName.clear();
                inVar = false;
                result += c;
            }
        } else {
            result += c;
        }
    }
    if (inVar) {
        result += get(varName);
    }
    return result;
}

} // namespace termidash
