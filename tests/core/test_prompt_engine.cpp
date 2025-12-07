#include <gtest/gtest.h>
#include "core/PromptEngine.hpp"
#include <filesystem>
#include <cstdlib>

using namespace termidash;
namespace fs = std::filesystem;

// ============================================================================
// Basic Functionality Tests
// ============================================================================

TEST(PromptEngine, Singleton) {
    PromptEngine& p1 = PromptEngine::instance();
    PromptEngine& p2 = PromptEngine::instance();
    EXPECT_EQ(&p1, &p2);
}

TEST(PromptEngine, SetGetPS1) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("test> ");
    EXPECT_EQ(engine.getPS1(), "test> ");
    
    // Restore
    engine.setPS1(original);
}

// ============================================================================
// Escape Sequence Tests
// ============================================================================

TEST(PromptEngine, Render_Username) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\u");
    std::string result = engine.render();
    
    // Should not be empty and should not contain backslash
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.find('\\'), std::string::npos);
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_Hostname) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\h");
    std::string result = engine.render();
    
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.find('\\'), std::string::npos);
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_CurrentDir) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\w");
    std::string result = engine.render();
    
    EXPECT_FALSE(result.empty());
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_CurrentDirBasename) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\W");
    std::string result = engine.render();
    
    EXPECT_FALSE(result.empty());
    // Basename should not contain path separators (usually)
    // But root or home might, so we just check it renders
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_PrivilegeIndicator) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\$");
    std::string result = engine.render();
    
    // Should be either $ or #
    EXPECT_TRUE(result == "$" || result == "#");
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_Time24h) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\t");
    std::string result = engine.render();
    
    // Should be in format HH:MM:SS
    EXPECT_EQ(result.size(), 8);
    EXPECT_EQ(result[2], ':');
    EXPECT_EQ(result[5], ':');
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_Date) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("\\d");
    std::string result = engine.render();
    
    // Should contain day and month
    EXPECT_FALSE(result.empty());
    EXPECT_GT(result.size(), 5);
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_Newline) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("line1\\nline2");
    std::string result = engine.render();
    
    EXPECT_EQ(result, "line1\nline2");
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_Backslash) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("a\\\\b");
    std::string result = engine.render();
    
    EXPECT_EQ(result, "a\\b");
    
    engine.setPS1(original);
}

// ============================================================================
// Complex Prompt Tests
// ============================================================================

TEST(PromptEngine, Render_ComplexPrompt) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    // Typical PS1 format: user@host:dir$
    engine.setPS1("\\u@\\h:\\w\\$ ");
    std::string result = engine.render();
    
    // Should contain @ and : and either $ or #
    EXPECT_NE(result.find('@'), std::string::npos);
    EXPECT_NE(result.find(':'), std::string::npos);
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_PlainText) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    engine.setPS1("simple> ");
    std::string result = engine.render();
    
    EXPECT_EQ(result, "simple> ");
    
    engine.setPS1(original);
}

TEST(PromptEngine, Render_EscapeCode) {
    PromptEngine& engine = PromptEngine::instance();
    std::string original = engine.getPS1();
    
    // Test escape character
    engine.setPS1("\\e[32mgreen\\e[0m");
    std::string result = engine.render();
    
    // \e should become escape character (0x1B)
    EXPECT_NE(result.find('\033'), std::string::npos);
    
    engine.setPS1(original);
}
