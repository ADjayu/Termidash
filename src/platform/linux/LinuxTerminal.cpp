#include "platform/linux/LinuxTerminal.hpp"
#include <iostream>
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

namespace termidash {
namespace platform {
namespace linux_platform {

LinuxTerminal::LinuxTerminal() {
    tcgetattr(STDIN_FILENO, &originalTermios);
}

LinuxTerminal::~LinuxTerminal() {
    disableRawMode();
}

char LinuxTerminal::readChar() {
    return getchar();
}

std::string LinuxTerminal::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void LinuxTerminal::write(const std::string& data) {
    std::cout << data << std::flush;
}

void LinuxTerminal::writeLine(const std::string& data) {
    std::cout << data << std::endl;
}

void LinuxTerminal::enableRawMode() {
    if (rawModeEnabled) return;
    struct termios raw = originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    rawModeEnabled = true;
}

void LinuxTerminal::disableRawMode() {
    if (!rawModeEnabled) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
    rawModeEnabled = false;
}

void LinuxTerminal::clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

int LinuxTerminal::getScreenWidth() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return 80; // Fallback
    }
    return ws.ws_col;
}

int LinuxTerminal::getScreenHeight() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_row == 0) {
        return 24; // Fallback
    }
    return ws.ws_row;
}

} // namespace linux_platform
} // namespace platform
} // namespace termidash
