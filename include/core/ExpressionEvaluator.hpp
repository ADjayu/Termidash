#pragma once
#include <string>
#include <vector>
#include <stdexcept>

namespace termidash {

class ExpressionEvaluator {
public:
    static long long evaluate(const std::string& expression);

private:
    static long long parseEquality(const std::string& expr, size_t& pos);
    static long long parseComparison(const std::string& expr, size_t& pos);
    static long long parseExpression(const std::string& expr, size_t& pos);
    static long long parseTerm(const std::string& expr, size_t& pos);
    static long long parseFactor(const std::string& expr, size_t& pos);
    static long long parseNumber(const std::string& expr, size_t& pos);
    static void skipWhitespace(const std::string& expr, size_t& pos);
};

} // namespace termidash
