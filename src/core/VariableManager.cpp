#include "core/VariableManager.hpp"
#include <cstdlib>

namespace termidash {

VariableManager& VariableManager::instance() {
    static VariableManager instance;
    return instance;
}

std::string VariableManager::get(const std::string& name) const {
    // Check scopes first (reverse order)
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto vit = it->find(name);
        if (vit != it->end()) {
            return vit->second;
        }
    }

    // Check local variables
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    
    // Fallback to environment variables
    const char* envVal = std::getenv(name.c_str());
    if (envVal) {
        return std::string(envVal);
    }
    
    return "";
}

bool VariableManager::has(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) return true;
    }

    if (variables.find(name) != variables.end()) {
        return true;
    }
    return std::getenv(name.c_str()) != nullptr;
}

void VariableManager::unset(const std::string& name) {
    // Unset from current scope if exists
    if (!scopes.empty()) {
        scopes.back().erase(name);
    }
    variables.erase(name);
}

std::map<std::string, std::string> VariableManager::getAll() const {
    std::map<std::string, std::string> all = variables;
    for (const auto& scope : scopes) {
        for (const auto& pair : scope) {
            all[pair.first] = pair.second;
        }
    }
    return all;
}

void VariableManager::pushScope() {
    scopes.push_back({});
}

void VariableManager::popScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void VariableManager::set(const std::string& name, const std::string& value) {
    if (!scopes.empty()) {
        scopes.back()[name] = value;
    } else {
        variables[name] = value;
    }
}

} // namespace termidash
