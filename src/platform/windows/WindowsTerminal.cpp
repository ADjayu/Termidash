#include "platform/windows/WindowsTerminal.hpp"
#include <conio.h>
#include <iostream>
#include <vector>

namespace termidash {
namespace platform {
namespace windows {

WindowsTerminal::WindowsTerminal() {
    hInput = GetStdHandle(STD_INPUT_HANDLE);
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hInput, &originalInputMode);
    GetConsoleMode(hOutput, &originalOutputMode);
}

WindowsTerminal::~WindowsTerminal() {
    disableRawMode();
}

char WindowsTerminal::readChar() {
    return _getch();
}

std::string WindowsTerminal::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void WindowsTerminal::write(const std::string& data) {
    DWORD written;
    WriteConsoleA(hOutput, data.c_str(), static_cast<DWORD>(data.size()), &written, nullptr);
}

void WindowsTerminal::writeLine(const std::string& data) {
    write(data + "\n");
}

void WindowsTerminal::enableRawMode() {
    DWORD mode = originalInputMode;
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hInput, mode);
}

void WindowsTerminal::disableRawMode() {
    SetConsoleMode(hInput, originalInputMode);
    SetConsoleMode(hOutput, originalOutputMode);
}

void WindowsTerminal::clearScreen() {
    COORD coord = {0, 0};
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    FillConsoleOutputCharacterA(hOutput, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(hOutput, coord);
}

int WindowsTerminal::getScreenWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int WindowsTerminal::getScreenHeight() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

} // namespace windows
} // namespace platform
} // namespace termidash
