/**
 * @file test_parser.cpp
 * @brief Unit tests for the Parser class
 */

#include <gtest/gtest.h>
#include "core/Parser.hpp"

using namespace termidash;

// ============================================================================
// Trim Tests
// ============================================================================

TEST(ParserTest, TrimRemovesLeadingWhitespace) {
    EXPECT_EQ(Parser::trim("  hello"), "hello");
    EXPECT_EQ(Parser::trim("\t\thello"), "hello");
}

TEST(ParserTest, TrimRemovesTrailingWhitespace) {
    EXPECT_EQ(Parser::trim("hello  "), "hello");
    EXPECT_EQ(Parser::trim("hello\t\t"), "hello");
}

TEST(ParserTest, TrimRemovesBothEnds) {
    EXPECT_EQ(Parser::trim("  hello  "), "hello");
    EXPECT_EQ(Parser::trim("\t hello \t"), "hello");
}

TEST(ParserTest, TrimEmptyString) {
    EXPECT_EQ(Parser::trim(""), "");
    EXPECT_EQ(Parser::trim("   "), "");
}

// ============================================================================
// SplitBatch Tests
// ============================================================================

TEST(ParserTest, SplitBatchSingleCommand) {
    auto result = Parser::splitBatch("echo hello");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].first, "echo hello");
    EXPECT_EQ(result[0].second, "");
}

TEST(ParserTest, SplitBatchWithSemicolon) {
    auto result = Parser::splitBatch("echo a; echo b");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].first, "echo a");
    EXPECT_EQ(result[0].second, ";");
    EXPECT_EQ(result[1].first, "echo b");
    EXPECT_EQ(result[1].second, "");
}

TEST(ParserTest, SplitBatchWithAnd) {
    auto result = Parser::splitBatch("cmd1 && cmd2");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].first, "cmd1");
    EXPECT_EQ(result[0].second, "&&");
    EXPECT_EQ(result[1].first, "cmd2");
}

TEST(ParserTest, SplitBatchWithOr) {
    auto result = Parser::splitBatch("cmd1 || cmd2");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].first, "cmd1");
    EXPECT_EQ(result[0].second, "||");
}

TEST(ParserTest, SplitBatchMultipleOperators) {
    auto result = Parser::splitBatch("a && b || c; d");
    EXPECT_EQ(result.size(), 4);
}

// ============================================================================
// Tokenize Tests
// ============================================================================

TEST(ParserTest, TokenizeSimple) {
    auto tokens = Parser::tokenize("echo hello world");
    EXPECT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello");
    EXPECT_EQ(tokens[2], "world");
}

TEST(ParserTest, TokenizeWithQuotes) {
    auto tokens = Parser::tokenize("echo \"hello world\"");
    EXPECT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "echo");
    EXPECT_EQ(tokens[1], "hello world");
}

TEST(ParserTest, TokenizeMultipleSpaces) {
    auto tokens = Parser::tokenize("echo   hello    world");
    EXPECT_EQ(tokens.size(), 3);
}

// ============================================================================
// ParseRedirection Tests
// ============================================================================

TEST(ParserTest, ParseRedirectionNoRedirection) {
    auto info = Parser::parseRedirection("echo hello");
    EXPECT_EQ(info.command, "echo hello");
    EXPECT_EQ(info.inFile, "");
    EXPECT_EQ(info.outFile, "");
    EXPECT_FALSE(info.isHereDoc);
}

TEST(ParserTest, ParseRedirectionOutputFile) {
    auto info = Parser::parseRedirection("echo hello > out.txt");
    EXPECT_EQ(info.command, "echo hello");
    EXPECT_EQ(info.outFile, "out.txt");
    EXPECT_FALSE(info.appendOut);
}

TEST(ParserTest, ParseRedirectionAppendOutput) {
    auto info = Parser::parseRedirection("echo hello >> out.txt");
    EXPECT_EQ(info.command, "echo hello");
    EXPECT_EQ(info.outFile, "out.txt");
    EXPECT_TRUE(info.appendOut);
}

TEST(ParserTest, ParseRedirectionInputFile) {
    auto info = Parser::parseRedirection("cat < input.txt");
    EXPECT_EQ(info.command, "cat");
    EXPECT_EQ(info.inFile, "input.txt");
}

TEST(ParserTest, ParseRedirectionStderr) {
    auto info = Parser::parseRedirection("cmd 2> error.txt");
    EXPECT_EQ(info.command, "cmd");
    EXPECT_EQ(info.errFile, "error.txt");
    EXPECT_FALSE(info.appendErr);
}

TEST(ParserTest, ParseRedirectionHereDoc) {
    auto info = Parser::parseRedirection("cat << EOF");
    EXPECT_EQ(info.command, "cat");
    EXPECT_TRUE(info.isHereDoc);
    EXPECT_EQ(info.hereDocDelim, "EOF");
}

TEST(ParserTest, ParseRedirectionBothOutput) {
    auto info = Parser::parseRedirection("cmd &> all.txt");
    EXPECT_EQ(info.command, "cmd");
    EXPECT_EQ(info.outFile, "all.txt");
    EXPECT_EQ(info.errFile, "all.txt");
}

// ============================================================================
// SplitPipelineOperators Tests
// ============================================================================

TEST(ParserTest, SplitPipelineSingleCommand) {
    auto segments = Parser::splitPipelineOperators("echo hello");
    EXPECT_EQ(segments.size(), 1);
    EXPECT_EQ(segments[0].cmd, "echo hello");
    EXPECT_FALSE(segments[0].trimBeforeNext);
}

TEST(ParserTest, SplitPipelineStandardPipe) {
    auto segments = Parser::splitPipelineOperators("cmd1 | cmd2");
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments[0].cmd, "cmd1");
    EXPECT_FALSE(segments[0].trimBeforeNext);
    EXPECT_EQ(segments[1].cmd, "cmd2");
}

TEST(ParserTest, SplitPipelineTrimPipe) {
    auto segments = Parser::splitPipelineOperators("cmd1 |> cmd2");
    EXPECT_EQ(segments.size(), 2);
    EXPECT_EQ(segments[0].cmd, "cmd1");
    EXPECT_TRUE(segments[0].trimBeforeNext);
    EXPECT_EQ(segments[1].cmd, "cmd2");
}

TEST(ParserTest, SplitPipelineMixed) {
    auto segments = Parser::splitPipelineOperators("a | b |> c | d");
    EXPECT_EQ(segments.size(), 4);
    EXPECT_FALSE(segments[0].trimBeforeNext);
    EXPECT_TRUE(segments[1].trimBeforeNext);
    EXPECT_FALSE(segments[2].trimBeforeNext);
}

// ============================================================================
// ApplyTrimToLines Tests
// ============================================================================

TEST(ParserTest, ApplyTrimToLinesTrimsEachLine) {
    std::string input = "  hello  \n  world  \n";
    std::string output = Parser::applyTrimToLines(input);
    EXPECT_EQ(output, "hello\nworld\n");
}

TEST(ParserTest, ApplyTrimToLinesEmptyLines) {
    std::string input = "hello\n   \nworld\n";
    std::string output = Parser::applyTrimToLines(input);
    EXPECT_EQ(output, "hello\nworld\n");
}
