#ifndef _WIN32  // Linux/macOS only

#include "core/BuiltIn/LinuxCommandHandler.hpp"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <unordered_set>
#include <algorithm>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <fstream>

namespace fs = std::filesystem;

namespace termidash {

bool LinuxCommandHandler::isCommand(const std::string& cmd) const {
    static const std::unordered_set<std::string> commands = {
        "ls", "cp", "mv", "chmod", "chown", "ln", "df", "free"
    };
    return commands.find(cmd) != commands.end();
}

bool LinuxCommandHandler::handle(const std::vector<std::string>& tokens) {
    ExecContext ctx(std::cin, std::cout, std::cerr);
    return handleWithContext(tokens, ctx) == 0;
}

int LinuxCommandHandler::handleWithContext(const std::vector<std::string>& tokens, ExecContext& ctx) {
    if (tokens.empty()) return 1;
    
    const std::string& cmd = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    
    if (cmd == "ls") return handleLs(args, ctx);
    if (cmd == "cp") return handleCp(args, ctx);
    if (cmd == "mv") return handleMv(args, ctx);
    if (cmd == "chmod") return handleChmod(args, ctx);
    if (cmd == "chown") return handleChown(args, ctx);
    if (cmd == "ln") return handleLn(args, ctx);
    if (cmd == "df") return handleDf(args, ctx);
    if (cmd == "free") return handleFree(args, ctx);
    
    return 1;
}

std::string LinuxCommandHandler::formatSize(uintmax_t bytes, bool humanReadable) const {
    if (!humanReadable) {
        return std::to_string(bytes);
    }
    
    const char* units[] = {"B", "K", "M", "G", "T", "P"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && unit < 5) {
        size /= 1024;
        ++unit;
    }
    
    std::ostringstream oss;
    if (unit == 0) {
        oss << bytes;
    } else {
        oss << std::fixed << std::setprecision(1) << size << units[unit];
    }
    return oss.str();
}

std::string LinuxCommandHandler::formatPermissions(unsigned int mode) const {
    std::string result;
    
    // File type
    if (S_ISDIR(mode)) result += 'd';
    else if (S_ISLNK(mode)) result += 'l';
    else if (S_ISBLK(mode)) result += 'b';
    else if (S_ISCHR(mode)) result += 'c';
    else if (S_ISFIFO(mode)) result += 'p';
    else if (S_ISSOCK(mode)) result += 's';
    else result += '-';
    
    // Owner permissions
    result += (mode & S_IRUSR) ? 'r' : '-';
    result += (mode & S_IWUSR) ? 'w' : '-';
    result += (mode & S_IXUSR) ? 'x' : '-';
    
    // Group permissions  
    result += (mode & S_IRGRP) ? 'r' : '-';
    result += (mode & S_IWGRP) ? 'w' : '-';
    result += (mode & S_IXGRP) ? 'x' : '-';
    
    // Other permissions
    result += (mode & S_IROTH) ? 'r' : '-';
    result += (mode & S_IWOTH) ? 'w' : '-';
    result += (mode & S_IXOTH) ? 'x' : '-';
    
    return result;
}

std::string LinuxCommandHandler::formatTime(std::time_t time) const {
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%b %d %H:%M", std::localtime(&time));
    return std::string(buffer);
}

int LinuxCommandHandler::handleLs(const std::vector<std::string>& args, ExecContext& ctx) {
    bool longFormat = false;
    bool showHidden = false;
    bool humanReadable = false;
    bool recursive = false;
    std::vector<std::string> paths;
    
    // Parse arguments
    for (const auto& arg : args) {
        if (arg[0] == '-') {
            for (size_t i = 1; i < arg.size(); ++i) {
                switch (arg[i]) {
                    case 'l': longFormat = true; break;
                    case 'a': showHidden = true; break;
                    case 'h': humanReadable = true; break;
                    case 'R': recursive = true; break;
                }
            }
        } else {
            paths.push_back(arg);
        }
    }
    
    if (paths.empty()) {
        paths.push_back(".");
    }
    
    for (const auto& path : paths) {
        try {
            if (!fs::exists(path)) {
                ctx.err << "ls: cannot access '" << path << "': No such file or directory\n";
                continue;
            }
            
            if (paths.size() > 1) {
                ctx.out << path << ":\n";
            }
            
            std::vector<fs::directory_entry> entries;
            for (const auto& entry : fs::directory_iterator(path)) {
                std::string name = entry.path().filename().string();
                if (!showHidden && !name.empty() && name[0] == '.') {
                    continue;
                }
                entries.push_back(entry);
            }
            
            // Sort by name
            std::sort(entries.begin(), entries.end(), 
                [](const fs::directory_entry& a, const fs::directory_entry& b) {
                    return a.path().filename() < b.path().filename();
                });
            
            for (const auto& entry : entries) {
                if (longFormat) {
                    struct stat st;
                    if (stat(entry.path().c_str(), &st) == 0) {
                        ctx.out << formatPermissions(st.st_mode) << " ";
                        ctx.out << std::setw(3) << st.st_nlink << " ";
                        
                        struct passwd* pw = getpwuid(st.st_uid);
                        ctx.out << std::setw(8) << (pw ? pw->pw_name : std::to_string(st.st_uid)) << " ";
                        
                        struct group* gr = getgrgid(st.st_gid);
                        ctx.out << std::setw(8) << (gr ? gr->gr_name : std::to_string(st.st_gid)) << " ";
                        
                        ctx.out << std::setw(8) << formatSize(st.st_size, humanReadable) << " ";
                        ctx.out << formatTime(st.st_mtime) << " ";
                    }
                }
                
                ctx.out << entry.path().filename().string();
                
                if (longFormat && fs::is_symlink(entry.path())) {
                    ctx.out << " -> " << fs::read_symlink(entry.path()).string();
                }
                
                ctx.out << "\n";
            }
            
        } catch (const fs::filesystem_error& e) {
            ctx.err << "ls: " << e.what() << "\n";
        }
    }
    
    return 0;
}

int LinuxCommandHandler::handleCp(const std::vector<std::string>& args, ExecContext& ctx) {
    bool recursive = false;
    bool force = false;
    std::vector<std::string> paths;
    
    for (const auto& arg : args) {
        if (arg[0] == '-') {
            for (size_t i = 1; i < arg.size(); ++i) {
                switch (arg[i]) {
                    case 'r': case 'R': recursive = true; break;
                    case 'f': force = true; break;
                }
            }
        } else {
            paths.push_back(arg);
        }
    }
    
    if (paths.size() < 2) {
        ctx.err << "cp: missing destination file operand\n";
        return 1;
    }
    
    std::string dest = paths.back();
    paths.pop_back();
    
    try {
        fs::copy_options opts = fs::copy_options::none;
        if (recursive) opts |= fs::copy_options::recursive;
        if (force) opts |= fs::copy_options::overwrite_existing;
        
        for (const auto& src : paths) {
            fs::copy(src, dest, opts);
        }
    } catch (const fs::filesystem_error& e) {
        ctx.err << "cp: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

int LinuxCommandHandler::handleMv(const std::vector<std::string>& args, ExecContext& ctx) {
    std::vector<std::string> paths;
    
    for (const auto& arg : args) {
        if (arg[0] != '-') {
            paths.push_back(arg);
        }
    }
    
    if (paths.size() < 2) {
        ctx.err << "mv: missing destination file operand\n";
        return 1;
    }
    
    std::string dest = paths.back();
    paths.pop_back();
    
    try {
        for (const auto& src : paths) {
            fs::rename(src, dest);
        }
    } catch (const fs::filesystem_error& e) {
        ctx.err << "mv: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

int LinuxCommandHandler::handleChmod(const std::vector<std::string>& args, ExecContext& ctx) {
    if (args.size() < 2) {
        ctx.err << "chmod: missing operand\n";
        return 1;
    }
    
    std::string modeStr = args[0];
    unsigned int mode = 0;
    
    // Parse octal mode
    try {
        mode = std::stoul(modeStr, nullptr, 8);
    } catch (...) {
        ctx.err << "chmod: invalid mode: " << modeStr << "\n";
        return 1;
    }
    
    for (size_t i = 1; i < args.size(); ++i) {
        if (chmod(args[i].c_str(), mode) != 0) {
            ctx.err << "chmod: cannot change permissions of '" << args[i] << "'\n";
            return 1;
        }
    }
    
    return 0;
}

int LinuxCommandHandler::handleChown(const std::vector<std::string>& args, ExecContext& ctx) {
    if (args.size() < 2) {
        ctx.err << "chown: missing operand\n";
        return 1;
    }
    
    std::string ownerSpec = args[0];
    uid_t uid = -1;
    gid_t gid = -1;
    
    size_t colonPos = ownerSpec.find(':');
    if (colonPos != std::string::npos) {
        std::string user = ownerSpec.substr(0, colonPos);
        std::string group = ownerSpec.substr(colonPos + 1);
        
        if (!user.empty()) {
            struct passwd* pw = getpwnam(user.c_str());
            if (pw) uid = pw->pw_uid;
        }
        if (!group.empty()) {
            struct group* gr = getgrnam(group.c_str());
            if (gr) gid = gr->gr_gid;
        }
    } else {
        struct passwd* pw = getpwnam(ownerSpec.c_str());
        if (pw) uid = pw->pw_uid;
    }
    
    for (size_t i = 1; i < args.size(); ++i) {
        if (chown(args[i].c_str(), uid, gid) != 0) {
            ctx.err << "chown: cannot change ownership of '" << args[i] << "'\n";
            return 1;
        }
    }
    
    return 0;
}

int LinuxCommandHandler::handleLn(const std::vector<std::string>& args, ExecContext& ctx) {
    bool symbolic = false;
    std::vector<std::string> paths;
    
    for (const auto& arg : args) {
        if (arg == "-s") {
            symbolic = true;
        } else if (arg[0] != '-') {
            paths.push_back(arg);
        }
    }
    
    if (paths.size() < 2) {
        ctx.err << "ln: missing destination file operand\n";
        return 1;
    }
    
    try {
        if (symbolic) {
            fs::create_symlink(paths[0], paths[1]);
        } else {
            fs::create_hard_link(paths[0], paths[1]);
        }
    } catch (const fs::filesystem_error& e) {
        ctx.err << "ln: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

int LinuxCommandHandler::handleDf(const std::vector<std::string>& args, ExecContext& ctx) {
    bool humanReadable = false;
    
    for (const auto& arg : args) {
        if (arg == "-h") humanReadable = true;
    }
    
    ctx.out << std::setw(20) << std::left << "Filesystem"
            << std::setw(12) << std::right << "Size"
            << std::setw(12) << "Used"
            << std::setw(12) << "Avail"
            << std::setw(8) << "Use%"
            << " Mounted on\n";
    
    std::ifstream mounts("/proc/mounts");
    std::string line;
    std::unordered_set<std::string> seen;
    
    while (std::getline(mounts, line)) {
        std::istringstream iss(line);
        std::string device, mountpoint, fstype;
        iss >> device >> mountpoint >> fstype;
        
        // Skip non-real filesystems
        if (device[0] != '/' && fstype != "tmpfs") continue;
        if (seen.count(device)) continue;
        seen.insert(device);
        
        struct statvfs vfs;
        if (statvfs(mountpoint.c_str(), &vfs) == 0) {
            uintmax_t total = vfs.f_blocks * vfs.f_frsize;
            uintmax_t free = vfs.f_bfree * vfs.f_frsize;
            uintmax_t avail = vfs.f_bavail * vfs.f_frsize;
            uintmax_t used = total - free;
            int usePercent = total > 0 ? (used * 100 / total) : 0;
            
            ctx.out << std::setw(20) << std::left << device
                    << std::setw(12) << std::right << formatSize(total, humanReadable)
                    << std::setw(12) << formatSize(used, humanReadable)
                    << std::setw(12) << formatSize(avail, humanReadable)
                    << std::setw(7) << usePercent << "%"
                    << " " << mountpoint << "\n";
        }
    }
    
    return 0;
}

int LinuxCommandHandler::handleFree(const std::vector<std::string>& args, ExecContext& ctx) {
    bool humanReadable = false;
    bool megabytes = false;
    bool gigabytes = false;
    
    for (const auto& arg : args) {
        if (arg == "-h") humanReadable = true;
        else if (arg == "-m") megabytes = true;
        else if (arg == "-g") gigabytes = true;
    }
    
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    uintmax_t total = 0, free = 0, available = 0, buffers = 0, cached = 0;
    uintmax_t swapTotal = 0, swapFree = 0;
    
    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        uintmax_t value;
        iss >> key >> value;
        
        if (key == "MemTotal:") total = value * 1024;
        else if (key == "MemFree:") free = value * 1024;
        else if (key == "MemAvailable:") available = value * 1024;
        else if (key == "Buffers:") buffers = value * 1024;
        else if (key == "Cached:") cached = value * 1024;
        else if (key == "SwapTotal:") swapTotal = value * 1024;
        else if (key == "SwapFree:") swapFree = value * 1024;
    }
    
    uintmax_t used = total - free - buffers - cached;
    
    auto format = [&](uintmax_t bytes) -> std::string {
        if (humanReadable) return formatSize(bytes, true);
        if (gigabytes) return std::to_string(bytes / (1024*1024*1024)) + "G";
        if (megabytes) return std::to_string(bytes / (1024*1024)) + "M";
        return std::to_string(bytes / 1024);  // Default: KB
    };
    
    ctx.out << std::setw(8) << "" 
            << std::setw(12) << "total"
            << std::setw(12) << "used"
            << std::setw(12) << "free"
            << std::setw(12) << "shared"
            << std::setw(12) << "buff/cache"
            << std::setw(12) << "available\n";
    
    ctx.out << std::setw(8) << std::left << "Mem:"
            << std::setw(12) << std::right << format(total)
            << std::setw(12) << format(used)
            << std::setw(12) << format(free)
            << std::setw(12) << "0"
            << std::setw(12) << format(buffers + cached)
            << std::setw(12) << format(available) << "\n";
    
    ctx.out << std::setw(8) << std::left << "Swap:"
            << std::setw(12) << std::right << format(swapTotal)
            << std::setw(12) << format(swapTotal - swapFree)
            << std::setw(12) << format(swapFree) << "\n";
    
    return 0;
}

} // namespace termidash

#endif // _WIN32
