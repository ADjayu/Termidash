#pragma once
#include "platform/interfaces/ITerminal.hpp"
#define NOMINMAX
#include <windows.h>

namespace termidash {
namespace platform {
namespace windows {

class WindowsTerminal : public ITerminal {
public:
    WindowsTerminal();
    ~WindowsTerminal() override;

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
    HANDLE hInput;
    HANDLE hOutput;
    DWORD originalInputMode;
    DWORD originalOutputMode;
};

} // namespace windows
} // namespace platform
} // namespace termidash
