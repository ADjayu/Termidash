#pragma once
#include <string>

namespace termidash {
namespace platform {

class ITerminal {
public:
    virtual ~ITerminal() = default;

    // Basic I/O
    virtual char readChar() = 0;
    virtual std::string readLine() = 0;
    virtual void write(const std::string& data) = 0;
    virtual void writeLine(const std::string& data) = 0;

    // Mode control
    virtual void enableRawMode() = 0;
    virtual void disableRawMode() = 0;

    // Screen control
    virtual void clearScreen() = 0;
    virtual int getScreenWidth() = 0;
    virtual int getScreenHeight() = 0;
};

} // namespace platform
} // namespace termidash
