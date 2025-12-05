#include "core/InputHandler.hpp"
#include <cctype>

namespace termidash {

std::string InputHandler::readLine(
    platform::ITerminal* terminal,
    const std::vector<std::string>& history,
    size_t& historyIndex,
    std::function<std::vector<std::string>(const std::string&)> completionGenerator
) {
    std::string buffer;
    size_t cursor = 0;
    
    while (true) {
        int ch = terminal->readChar();
        
        if (ch == 13) { // Enter
            terminal->write("\n");
            break;
        } else if (ch == 8) { // Backspace
            if (cursor > 0) {
                buffer.erase(cursor - 1, 1);
                cursor--;
                terminal->write("\b \b");
            }
        } else if (ch == 9) { // Tab
            size_t pos = buffer.find_last_of(" \t");
            std::string prefix = (pos == std::string::npos) ? buffer : buffer.substr(pos + 1);
            auto matches = CompletionEngine::complete(prefix, completionGenerator);
            
            if (matches.size() == 1) {
                std::string addition = matches[0].substr(prefix.size());
                buffer += addition;
                terminal->write(addition);
                cursor += addition.size();
            } else if (matches.size() > 1) {
                terminal->write("\n");
                for (size_t i = 0; i < matches.size() && i < 10; ++i) {
                    terminal->write(matches[i] + " ");
                }
                terminal->write("\n> " + buffer);
            }
        } else if (ch == 224) { // Arrow keys (Windows)
            int next = terminal->readChar();
            if (next == 72) { // Up
                if (historyIndex > 0) {
                    historyIndex--;
                    for (size_t i = 0; i < buffer.size(); ++i)
                        terminal->write("\b \b");
                    buffer = history[historyIndex];
                    cursor = buffer.size();
                    terminal->write(buffer);
                }
            } else if (next == 80) { // Down
                if (historyIndex + 1 < history.size()) {
                    historyIndex++;
                    for (size_t i = 0; i < buffer.size(); ++i)
                        terminal->write("\b \b");
                    buffer = history[historyIndex];
                    cursor = buffer.size();
                    terminal->write(buffer);
                } else {
                    for (size_t i = 0; i < buffer.size(); ++i)
                        terminal->write("\b \b");
                    buffer.clear();
                    cursor = 0;
                }
            }
        } else if (std::isprint(ch)) {
            buffer.insert(buffer.begin() + cursor, static_cast<char>(ch));
            terminal->write(std::string(1, static_cast<char>(ch)));
            cursor++;
        }
    }
    
    historyIndex = history.size();
    return buffer;
}

} // namespace termidash
