#include "core/Parser.hpp"

namespace termidash {

std::string Parser::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::pair<std::string, std::string>> Parser::splitBatch(const std::string& input) {
    std::vector<std::pair<std::string, std::string>> result;
    size_t i = 0;
    std::string current;
    while (i < input.size()) {
        if (i + 1 < input.size()) {
            std::string two = input.substr(i, 2);
            if (two == "&&" || two == "||") {
                result.emplace_back(trim(current), two);
                current.clear();
                i += 2;
                continue;
            }
        }
        if (input[i] == ';') {
            result.emplace_back(trim(current), ";");
            current.clear();
            i++;
            continue;
        }
        current += input[i++];
    }
    if (!current.empty()) {
        result.emplace_back(trim(current), "");
    }
    return result;
}

std::vector<std::string> Parser::tokenize(const std::string& cmd) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    for (char c : cmd) {
        if (c == '\"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

Parser::RedirectionInfo Parser::parseRedirection(const std::string& cmd) {
    RedirectionInfo info;
    
    std::vector<std::string> toks;
    // Split respecting quotes
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < cmd.size(); ++i) {
        char c = cmd[i];
        if (c == '\"') {
            inQuotes = !inQuotes;
            cur.push_back(c);
            continue;
        }
        if (!inQuotes && (c == ' ' || c == '\t')) {
            if (!cur.empty()) {
                toks.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty())
        toks.push_back(cur);

    for (size_t i = 0; i < toks.size(); ++i) {
        const std::string& t = toks[i];
        if (t == "<") {
            if (i + 1 < toks.size())
                info.inFile = toks[++i];
        } else if (t == "<<") {
            if (i + 1 < toks.size()) {
                info.hereDocDelim = toks[++i];
                info.isHereDoc = true;
            }
        } else if (t == ">>") {
            if (i + 1 < toks.size()) {
                info.outFile = toks[++i];
                info.appendOut = true;
            }
        } else if (t == ">" || t == "1>") {
            if (i + 1 < toks.size()) {
                info.outFile = toks[++i];
                info.appendOut = false;
            }
        } else if (t == "2>") {
            if (i + 1 < toks.size()) {
                info.errFile = toks[++i];
                info.appendErr = false;
            }
        } else if (t == "2>>") {
            if (i + 1 < toks.size()) {
                info.errFile = toks[++i];
                info.appendErr = true;
            }
        } else if (t == "&>" || t == ">&") {
            if (i + 1 < toks.size()) {
                info.outFile = toks[++i];
                info.errFile = info.outFile;
                info.appendOut = false;
                info.appendErr = false;
            }
        } else if (t == "&>>") {
            if (i + 1 < toks.size()) {
                info.outFile = toks[++i];
                info.errFile = info.outFile;
                info.appendOut = true;
                info.appendErr = true;
            }
        } else {
            if (!info.command.empty())
                info.command += ' ';
            info.command += t;
        }
    }
    
    return info;
}

std::vector<Parser::PipelineSegment> Parser::splitPipelineOperators(const std::string& line) {
    std::vector<PipelineSegment> segments;
    size_t pos = 0;
    std::string current;
    while (pos < line.size()) {
        if (pos + 1 < line.size() && line.substr(pos, 2) == "|>") {
            segments.push_back({trim(current), true});
            current.clear();
            pos += 2;
        } else if (line[pos] == '|') {
            segments.push_back({trim(current), false});
            current.clear();
            pos++;
        } else {
            current += line[pos++];
        }
    }
    if (!current.empty()) {
        segments.push_back({trim(current), false});
    }
    return segments;
}

std::string Parser::applyTrimToLines(const std::string& input) {
    std::string out;
    std::string line;
    size_t start = 0;
    while (start < input.size()) {
        size_t npos = input.find('\n', start);
        if (npos == std::string::npos)
            npos = input.size();
        line = input.substr(start, npos - start);
        size_t s = line.find_first_not_of(" \t");
        size_t e = line.find_last_not_of(" \t");
        if (s != std::string::npos && e != std::string::npos && e >= s) {
            out += line.substr(s, e - s + 1);
            out += '\n';
        }
        start = (npos == input.size()) ? npos : npos + 1;
    }
    return out;
}

} // namespace termidash
