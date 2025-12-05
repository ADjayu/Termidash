#pragma once
#include "platform/interfaces/ITerminal.hpp"
#include <termios.h>
#include <unistd.h>

namespace termidash {
namespace platform {
namespace linux_platform { // Avoid namespace collision with 'linux' macro if any

class LinuxTerminal : public ITerminal {
public:
    LinuxTerminal();
    ~LinuxTerminal() override;

    char readChar() override;
    std::string readLine() override;
    void write(const std::string& data) override;
    void writeLine(const std::string& data) override;

    void enableRawMode() override;
    void disableRawMode() override;

    void clearScreen() override;
    int getScreenWidth() override;
    int getScreenHeight() override;

private:
    struct termios originalTermios;
    bool rawModeEnabled = false;
};

} // namespace linux_platform
} // namespace platform
} // namespace termidash
