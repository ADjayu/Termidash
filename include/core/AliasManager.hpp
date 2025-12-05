#ifndef ALIAS_MANAGER_HPP
#define ALIAS_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace termidash {

class AliasManager {
public:
    static AliasManager& instance();

    void set(const std::string& name, const std::string& value);
    void unset(const std::string& name);
    std::string get(const std::string& name) const;
    std::unordered_map<std::string, std::string> getAll() const;
    bool has(const std::string& name) const;

private:
    AliasManager() = default;
    std::unordered_map<std::string, std::string> aliases;
};

} // namespace termidash

#endif // ALIAS_MANAGER_HPP
