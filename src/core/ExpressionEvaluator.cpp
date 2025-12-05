#include "core/ExpressionEvaluator.hpp"
#include <cctype>
#include <cmath>

namespace termidash {

long long ExpressionEvaluator::evaluate(const std::string& expression) {
    size_t pos = 0;
    return parseEquality(expression, pos);
}

void ExpressionEvaluator::skipWhitespace(const std::string& expr, size_t& pos) {
    while (pos < expr.length() && std::isspace(expr[pos])) {
        pos++;
    }
}

long long ExpressionEvaluator::parseEquality(const std::string& expr, size_t& pos) {
    long long left = parseComparison(expr, pos);
    skipWhitespace(expr, pos);
    while (pos < expr.length()) {
        if (expr.substr(pos, 2) == "==") {
            pos += 2;
            long long right = parseComparison(expr, pos);
            left = (left == right);
        } else if (expr.substr(pos, 2) == "!=") {
            pos += 2;
            long long right = parseComparison(expr, pos);
            left = (left != right);
        } else {
            break;
        }
    }
    return left;
}

long long ExpressionEvaluator::parseComparison(const std::string& expr, size_t& pos) {
    long long left = parseExpression(expr, pos);
    skipWhitespace(expr, pos);
    while (pos < expr.length()) {
        if (expr.substr(pos, 2) == "<=") {
            pos += 2;
            long long right = parseExpression(expr, pos);
            left = (left <= right);
        } else if (expr.substr(pos, 2) == ">=") {
            pos += 2;
            long long right = parseExpression(expr, pos);
            left = (left >= right);
        } else if (expr[pos] == '<') {
            pos++;
            long long right = parseExpression(expr, pos);
            left = (left < right);
        } else if (expr[pos] == '>') {
            pos++;
            long long right = parseExpression(expr, pos);
            left = (left > right);
        } else {
            break;
        }
    }
    return left;
}

long long ExpressionEvaluator::parseExpression(const std::string& expr, size_t& pos) {
    long long left = parseTerm(expr, pos);
    skipWhitespace(expr, pos);
    while (pos < expr.length()) {
        if (expr[pos] == '+') {
            pos++;
            long long right = parseTerm(expr, pos);
            left += right;
        } else if (expr[pos] == '-') {
            pos++;
            long long right = parseTerm(expr, pos);
            left -= right;
        } else {
            break;
        }
    }
    return left;
}

long long ExpressionEvaluator::parseTerm(const std::string& expr, size_t& pos) {
    long long left = parseFactor(expr, pos);
    skipWhitespace(expr, pos);
    while (pos < expr.length()) {
        if (expr[pos] == '*') {
            pos++;
            long long right = parseFactor(expr, pos);
            left *= right;
        } else if (expr[pos] == '/') {
            pos++;
            long long right = parseFactor(expr, pos);
            if (right == 0) throw std::runtime_error("Division by zero");
            left /= right;
        } else {
            break;
        }
    }
    return left;
}

long long ExpressionEvaluator::parseFactor(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    if (pos >= expr.length()) throw std::runtime_error("Unexpected end of expression");

    if (expr[pos] == '(') {
        pos++;
        long long val = parseEquality(expr, pos);
        skipWhitespace(expr, pos);
        if (pos >= expr.length() || expr[pos] != ')') {
            throw std::runtime_error("Mismatched parentheses");
        }
        pos++;
        return val;
    } else if (std::isdigit(expr[pos]) || expr[pos] == '-') {
        return parseNumber(expr, pos);
    } else {
        throw std::runtime_error("Invalid character in expression");
    }
}

long long ExpressionEvaluator::parseNumber(const std::string& expr, size_t& pos) {
    skipWhitespace(expr, pos);
    size_t start = pos;
    if (expr[pos] == '-') pos++;
    while (pos < expr.length() && std::isdigit(expr[pos])) {
        pos++;
    }
    if (start == pos) throw std::runtime_error("Expected number");
    return std::stoll(expr.substr(start, pos - start));
}

} // namespace termidash
