#include "core/AliasManager.hpp"

namespace termidash {

AliasManager& AliasManager::instance() {
    static AliasManager instance;
    return instance;
}

void AliasManager::set(const std::string& name, const std::string& value) {
    aliases[name] = value;
}

void AliasManager::unset(const std::string& name) {
    aliases.erase(name);
}

std::string AliasManager::get(const std::string& name) const {
    auto it = aliases.find(name);
    if (it != aliases.end()) {
        return it->second;
    }
    return "";
}

std::unordered_map<std::string, std::string> AliasManager::getAll() const {
    return aliases;
}

bool AliasManager::has(const std::string& name) const {
    return aliases.find(name) != aliases.end();
}

} // namespace termidash
