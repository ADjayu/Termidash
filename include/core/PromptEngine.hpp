#pragma once
#include <string>

namespace termidash {

/**
 * PromptEngine - Handles PS1-style prompt customization
 * 
 * Escape sequences:
 *   \u  - Username
 *   \h  - Hostname (short)
 *   \H  - Hostname (full)
 *   \w  - Current directory (full path, ~ for home)
 *   \W  - Current directory (basename only)
 *   \$  - '#' for root/admin, '$' for normal user
 *   \t  - Time in HH:MM:SS
 *   \T  - Time in 12-hour format
 *   \d  - Date (e.g., "Mon Dec 05")
 *   \n  - Newline
 *   \r  - Carriage return
 *   \\  - Literal backslash
 *   \e  - Escape character (for ANSI codes)
 *   \[  - Begin sequence of non-printing characters
 *   \]  - End sequence of non-printing characters
 */
class PromptEngine {
public:
    /**
     * Get the singleton instance.
     */
    static PromptEngine& instance();
    
    /**
     * Render the current prompt string by expanding escape sequences.
     * @return The rendered prompt ready for display
     */
    std::string render() const;
    
    /**
     * Set the PS1 prompt format string.
     * @param format The PS1 format string with escape sequences
     */
    void setPS1(const std::string& format);
    
    /**
     * Get the current PS1 format string.
     */
    const std::string& getPS1() const;
    
    /**
     * Set the default prompt (used if PS1 is not set).
     */
    void setDefaultPrompt(const std::string& prompt);
    
private:
    PromptEngine();
    PromptEngine(const PromptEngine&) = delete;
    PromptEngine& operator=(const PromptEngine&) = delete;
    
    std::string ps1_;
    std::string defaultPrompt_;
    
    /**
     * Expand a single escape sequence starting at position.
     * @param format The format string
     * @param pos Position of the backslash
     * @param consumed Number of characters consumed (output)
     * @return The expanded text
     */
    std::string expandEscape(const std::string& format, size_t pos, size_t& consumed) const;
    
    /**
     * Get the current username.
     */
    std::string getUsername() const;
    
    /**
     * Get the hostname.
     * @param full If true, return full hostname; otherwise short hostname
     */
    std::string getHostname(bool full = false) const;
    
    /**
     * Get the current working directory.
     * @param basename If true, return only the basename
     */
    std::string getCurrentDirectory(bool basename = false) const;
    
    /**
     * Check if current user has admin/root privileges.
     */
    bool isAdmin() const;
    
    /**
     * Get the current time in specified format.
     * @param format24h If true, use 24-hour format
     */
    std::string getTime(bool format24h = true) const;
    
    /**
     * Get the current date.
     */
    std::string getDate() const;
};

} // namespace termidash
