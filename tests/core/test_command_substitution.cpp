#include <gtest/gtest.h>
#include "core/CommandSubstitution.hpp"
#include <map>

using namespace termidash;

// Mock executor that returns predefined outputs
class MockExecutor {
public:
    std::map<std::string, std::string> outputs;
    
    std::string operator()(const std::string& cmd) {
        auto it = outputs.find(cmd);
        if (it != outputs.end()) {
            return it->second;
        }
        return "[unknown: " + cmd + "]";
    }
};

// ============================================================================
// hasSubstitution Tests
// ============================================================================

TEST(CommandSubstitution, HasSubstitution_DollarParen) {
    EXPECT_TRUE(CommandSubstitution::hasSubstitution("$(echo hello)"));
    EXPECT_TRUE(CommandSubstitution::hasSubstitution("prefix $(cmd) suffix"));
    EXPECT_TRUE(CommandSubstitution::hasSubstitution("$(nested $(inner))"));
}

TEST(CommandSubstitution, HasSubstitution_Backticks) {
    EXPECT_TRUE(CommandSubstitution::hasSubstitution("`echo hello`"));
    EXPECT_TRUE(CommandSubstitution::hasSubstitution("prefix `cmd` suffix"));
}

TEST(CommandSubstitution, HasSubstitution_NoPattern) {
    EXPECT_FALSE(CommandSubstitution::hasSubstitution("echo hello"));
    EXPECT_FALSE(CommandSubstitution::hasSubstitution("$VAR"));
    EXPECT_FALSE(CommandSubstitution::hasSubstitution("plain text"));
    EXPECT_FALSE(CommandSubstitution::hasSubstitution("$((1+2))")); // arithmetic, not command
}

// ============================================================================
// Basic $() Expansion Tests
// ============================================================================

TEST(CommandSubstitution, Expand_BasicDollarParen) {
    MockExecutor executor;
    executor.outputs["echo hello"] = "hello\n";
    
    std::string result = CommandSubstitution::expand("$(echo hello)", executor);
    EXPECT_EQ(result, "hello");
}

TEST(CommandSubstitution, Expand_WithPrefix) {
    MockExecutor executor;
    executor.outputs["pwd"] = "/home/user\n";
    
    std::string result = CommandSubstitution::expand("Current: $(pwd)", executor);
    EXPECT_EQ(result, "Current: /home/user");
}

TEST(CommandSubstitution, Expand_WithSuffix) {
    MockExecutor executor;
    executor.outputs["date"] = "2024-01-01\n";
    
    std::string result = CommandSubstitution::expand("$(date) is today", executor);
    EXPECT_EQ(result, "2024-01-01 is today");
}

TEST(CommandSubstitution, Expand_Multiple) {
    MockExecutor executor;
    executor.outputs["cmd1"] = "A\n";
    executor.outputs["cmd2"] = "B\n";
    
    std::string result = CommandSubstitution::expand("$(cmd1) and $(cmd2)", executor);
    EXPECT_EQ(result, "A and B");
}

// ============================================================================
// Backtick Expansion Tests
// ============================================================================

TEST(CommandSubstitution, Expand_Backticks) {
    MockExecutor executor;
    executor.outputs["echo test"] = "test\n";
    
    std::string result = CommandSubstitution::expand("`echo test`", executor);
    EXPECT_EQ(result, "test");
}

TEST(CommandSubstitution, Expand_BackticksWithContext) {
    MockExecutor executor;
    executor.outputs["ls"] = "file1 file2\n";
    
    std::string result = CommandSubstitution::expand("Files: `ls`", executor);
    EXPECT_EQ(result, "Files: file1 file2");
}

// ============================================================================
// Nested Substitution Tests
// ============================================================================

TEST(CommandSubstitution, Expand_Nested) {
    MockExecutor executor;
    executor.outputs["pwd"] = "/home/user\n";
    executor.outputs["dirname /home/user"] = "/home\n";
    
    std::string result = CommandSubstitution::expand("$(dirname $(pwd))", executor);
    EXPECT_EQ(result, "/home");
}

TEST(CommandSubstitution, Expand_DeeplyNested) {
    MockExecutor executor;
    executor.outputs["echo a"] = "a\n";
    executor.outputs["echo a b"] = "a b\n";
    executor.outputs["echo a b c"] = "a b c\n";
    
    std::string result = CommandSubstitution::expand("$(echo $(echo a) b)", executor);
    EXPECT_EQ(result, "a b");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(CommandSubstitution, Expand_NoSubstitution) {
    MockExecutor executor;
    
    std::string result = CommandSubstitution::expand("plain text", executor);
    EXPECT_EQ(result, "plain text");
}

TEST(CommandSubstitution, Expand_EmptyCommand) {
    MockExecutor executor;
    executor.outputs[""] = "";
    
    std::string result = CommandSubstitution::expand("$()", executor);
    EXPECT_EQ(result, "");
}

TEST(CommandSubstitution, Expand_UnmatchedParen) {
    MockExecutor executor;
    
    // Unmatched $( should be kept as-is
    std::string result = CommandSubstitution::expand("$(incomplete", executor);
    EXPECT_EQ(result, "$(incomplete");
}

TEST(CommandSubstitution, Expand_TrailingNewlines) {
    MockExecutor executor;
    executor.outputs["multi"] = "line1\nline2\n\n\n";
    
    std::string result = CommandSubstitution::expand("$(multi)", executor);
    EXPECT_EQ(result, "line1\nline2");
}

TEST(CommandSubstitution, Expand_WindowsLineEndings) {
    MockExecutor executor;
    executor.outputs["win"] = "output\r\n";
    
    std::string result = CommandSubstitution::expand("$(win)", executor);
    EXPECT_EQ(result, "output");
}

TEST(CommandSubstitution, Expand_QuotedContent) {
    MockExecutor executor;
    executor.outputs["echo \"hello world\""] = "hello world\n";
    
    std::string result = CommandSubstitution::expand("$(echo \"hello world\")", executor);
    EXPECT_EQ(result, "hello world");
}

// ============================================================================
// Complex Scenarios
// ============================================================================

TEST(CommandSubstitution, Expand_MixedSyntax) {
    MockExecutor executor;
    executor.outputs["cmd1"] = "A\n";
    executor.outputs["cmd2"] = "B\n";
    
    std::string result = CommandSubstitution::expand("$(cmd1) and `cmd2`", executor);
    EXPECT_EQ(result, "A and B");
}

TEST(CommandSubstitution, Expand_InQuotes) {
    MockExecutor executor;
    executor.outputs["whoami"] = "admin\n";
    
    // Substitution inside double quotes should still work
    std::string input = "User: $(whoami)";
    std::string result = CommandSubstitution::expand(input, executor);
    EXPECT_EQ(result, "User: admin");
}
