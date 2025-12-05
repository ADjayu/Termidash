/**
 * @file test_completion_engine.cpp
 * @brief Unit tests for the CompletionEngine class
 */

#include <gtest/gtest.h>
#include "core/CompletionEngine.hpp"

using namespace termidash;

// ============================================================================
// LCS Length Tests
// ============================================================================

TEST(CompletionEngineTest, LcsLengthIdenticalStrings) {
    EXPECT_EQ(CompletionEngine::lcsLength("hello", "hello"), 5);
}

TEST(CompletionEngineTest, LcsLengthNoCommon) {
    EXPECT_EQ(CompletionEngine::lcsLength("abc", "xyz"), 0);
}

TEST(CompletionEngineTest, LcsLengthPartialMatch) {
    EXPECT_EQ(CompletionEngine::lcsLength("abcde", "ace"), 3);
}

TEST(CompletionEngineTest, LcsLengthEmptyString) {
    EXPECT_EQ(CompletionEngine::lcsLength("", "hello"), 0);
    EXPECT_EQ(CompletionEngine::lcsLength("hello", ""), 0);
}

TEST(CompletionEngineTest, LcsLengthOneChar) {
    EXPECT_EQ(CompletionEngine::lcsLength("a", "a"), 1);
    EXPECT_EQ(CompletionEngine::lcsLength("a", "b"), 0);
}

// ============================================================================
// Complete Tests
// ============================================================================

TEST(CompletionEngineTest, CompleteExactPrefixMatch) {
    auto generator = [](const std::string& prefix) {
        return std::vector<std::string>{"hello", "help", "world"};
    };
    
    auto results = CompletionEngine::complete("hel", generator);
    EXPECT_GE(results.size(), 2);
    // hello and help should be at top
    EXPECT_EQ(results[0], "hello");
    EXPECT_EQ(results[1], "help");
}

TEST(CompletionEngineTest, CompleteNoMatch) {
    auto generator = [](const std::string& prefix) {
        return std::vector<std::string>{"aaa", "bbb", "ccc"};
    };
    
    auto results = CompletionEngine::complete("xyz", generator);
    EXPECT_TRUE(results.empty());
}

TEST(CompletionEngineTest, CompleteSubstringMatch) {
    auto generator = [](const std::string& prefix) {
        return std::vector<std::string>{"foobar", "bazfoo"};
    };
    
    auto results = CompletionEngine::complete("foo", generator);
    EXPECT_EQ(results.size(), 2);
    // foobar (prefix) should rank higher than bazfoo (substring)
    EXPECT_EQ(results[0], "foobar");
    EXPECT_EQ(results[1], "bazfoo");
}

TEST(CompletionEngineTest, CompleteFuzzyMatch) {
    auto generator = [](const std::string& prefix) {
        return std::vector<std::string>{"makefile", "manifest"};
    };
    
    auto results = CompletionEngine::complete("mak", generator);
    // makefile should rank higher (prefix match)
    EXPECT_GE(results.size(), 1);
    EXPECT_EQ(results[0], "makefile");
}

TEST(CompletionEngineTest, CompleteDeduplicated) {
    auto generator = [](const std::string& prefix) {
        // Generator returns duplicates
        return std::vector<std::string>{"hello", "hello", "help"};
    };
    
    auto results = CompletionEngine::complete("hel", generator);
    // Should not have duplicates
    EXPECT_EQ(results.size(), 2);
}

TEST(CompletionEngineTest, CompleteEmptyPrefix) {
    auto generator = [](const std::string& prefix) {
        return std::vector<std::string>{"abc", "def", "ghi"};
    };
    
    auto results = CompletionEngine::complete("", generator);
    // Empty prefix should match nothing with our current logic
    // (LCS of empty string is 0, prefix match of empty is still true for all)
    // Actually, rfind("", 0) == 0 is true for all strings, so all should match!
    EXPECT_EQ(results.size(), 3);
}
