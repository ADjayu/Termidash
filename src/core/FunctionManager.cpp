#include "core/FunctionManager.hpp"

namespace termidash {

FunctionManager& FunctionManager::instance() {
    static FunctionManager instance;
    return instance;
}

void FunctionManager::define(const std::string& name, const std::vector<std::string>& body) {
    functions[name] = body;
}

bool FunctionManager::has(const std::string& name) const {
    return functions.find(name) != functions.end();
}

const std::vector<std::string>& FunctionManager::getBody(const std::string& name) const {
    static const std::vector<std::string> empty;
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second;
    }
    return empty;
}

void FunctionManager::unset(const std::string& name) {
    functions.erase(name);
}

std::map<std::string, std::vector<std::string>> FunctionManager::getAll() const {
    return functions;
}

} // namespace termidash
