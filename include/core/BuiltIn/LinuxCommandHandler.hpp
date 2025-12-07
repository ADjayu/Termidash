#pragma once
#ifndef _WIN32  // Linux/macOS only

#include <string>
#include <vector>
#include "core/ExecContext.hpp"

namespace termidash {

/**
 * LinuxCommandHandler - Implements Linux-specific built-in commands
 * 
 * Commands:
 *   ls    - Directory listing with -l, -a, -h, -R options
 *   cp    - Copy files/directories with -r, -f options
 *   mv    - Move/rename files
 *   chmod - Change file permissions
 *   chown - Change file ownership
 *   ln    - Create links (-s for symbolic)
 *   df    - Disk space usage
 *   free  - Memory usage
 */
class LinuxCommandHandler {
public:
    /**
     * Check if the command is handled by this handler.
     */
    bool isCommand(const std::string& cmd) const;
    
    /**
     * Handle a command with execution context for I/O.
     * @return Exit code (0 = success)
     */
    int handleWithContext(const std::vector<std::string>& tokens, ExecContext& ctx);
    
    /**
     * Handle a command using standard I/O.
     */
    bool handle(const std::vector<std::string>& tokens);
    
private:
    // Command implementations
    int handleLs(const std::vector<std::string>& args, ExecContext& ctx);
    int handleCp(const std::vector<std::string>& args, ExecContext& ctx);
    int handleMv(const std::vector<std::string>& args, ExecContext& ctx);
    int handleChmod(const std::vector<std::string>& args, ExecContext& ctx);
    int handleChown(const std::vector<std::string>& args, ExecContext& ctx);
    int handleLn(const std::vector<std::string>& args, ExecContext& ctx);
    int handleDf(const std::vector<std::string>& args, ExecContext& ctx);
    int handleFree(const std::vector<std::string>& args, ExecContext& ctx);
    
    // Helper functions
    std::string formatSize(uintmax_t bytes, bool humanReadable) const;
    std::string formatPermissions(unsigned int mode) const;
    std::string formatTime(std::time_t time) const;
};

} // namespace termidash

#endif // _WIN32
