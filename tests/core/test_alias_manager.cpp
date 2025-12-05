/**
 * @file test_alias_manager.cpp
 * @brief Unit tests for the AliasManager class
 */

#include <gtest/gtest.h>
#include "core/AliasManager.hpp"

using namespace termidash;

class AliasManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear any aliases from previous tests
        auto& am = AliasManager::instance();
        auto all = am.getAll();
        for (const auto& pair : all) {
            am.unset(pair.first);
        }
    }
    
    AliasManager& am() {
        return AliasManager::instance();
    }
};

// ============================================================================
// Basic Set/Get Tests
// ============================================================================

TEST_F(AliasManagerTest, CanSetAndGetAlias) {
    am().set("ll", "ls -la");
    EXPECT_EQ(am().get("ll"), "ls -la");
}

TEST_F(AliasManagerTest, HasReturnsTrueForExistingAlias) {
    am().set("myalias", "some command");
    EXPECT_TRUE(am().has("myalias"));
}

TEST_F(AliasManagerTest, HasReturnsFalseForNonExistingAlias) {
    EXPECT_FALSE(am().has("nonexistent"));
}

TEST_F(AliasManagerTest, GetReturnsEmptyForNonExistingAlias) {
    EXPECT_EQ(am().get("nonexistent"), "");
}

TEST_F(AliasManagerTest, CanOverwriteAlias) {
    am().set("cmd", "first command");
    am().set("cmd", "second command");
    EXPECT_EQ(am().get("cmd"), "second command");
}

// ============================================================================
// Unset Tests
// ============================================================================

TEST_F(AliasManagerTest, UnsetRemovesAlias) {
    am().set("to_remove", "command");
    EXPECT_TRUE(am().has("to_remove"));
    
    am().unset("to_remove");
    EXPECT_FALSE(am().has("to_remove"));
}

TEST_F(AliasManagerTest, UnsetNonexistentDoesNotThrow) {
    EXPECT_NO_THROW(am().unset("never_set"));
}

// ============================================================================
// GetAll Tests
// ============================================================================

TEST_F(AliasManagerTest, GetAllReturnsAllAliases) {
    am().set("alias1", "cmd1");
    am().set("alias2", "cmd2");
    am().set("alias3", "cmd3");
    
    auto all = am().getAll();
    EXPECT_EQ(all.size(), 3);
    EXPECT_EQ(all["alias1"], "cmd1");
    EXPECT_EQ(all["alias2"], "cmd2");
    EXPECT_EQ(all["alias3"], "cmd3");
}

TEST_F(AliasManagerTest, GetAllReturnsEmptyWhenNoAliases) {
    auto all = am().getAll();
    EXPECT_TRUE(all.empty());
}

// ============================================================================
// Typical Usage Patterns
// ============================================================================

TEST_F(AliasManagerTest, WorksWithTypicalAliases) {
    am().set("ll", "ls -la");
    am().set("la", "ls -A");
    am().set("grep", "grep --color=auto");
    am().set("..", "cd ..");
    
    EXPECT_EQ(am().get("ll"), "ls -la");
    EXPECT_EQ(am().get("la"), "ls -A");
    EXPECT_EQ(am().get("grep"), "grep --color=auto");
    EXPECT_EQ(am().get(".."), "cd ..");
}

TEST_F(AliasManagerTest, WorksWithComplexCommands) {
    am().set("update", "apt-get update && apt-get upgrade -y");
    am().set("gitlog", "git log --oneline --graph --all");
    
    EXPECT_EQ(am().get("update"), "apt-get update && apt-get upgrade -y");
    EXPECT_EQ(am().get("gitlog"), "git log --oneline --graph --all");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(AliasManagerTest, HandlesEmptyAliasName) {
    am().set("", "empty_name_alias");
    EXPECT_EQ(am().get(""), "empty_name_alias");
    EXPECT_TRUE(am().has(""));
}

TEST_F(AliasManagerTest, HandlesEmptyCommand) {
    am().set("empty", "");
    EXPECT_TRUE(am().has("empty"));
    EXPECT_EQ(am().get("empty"), "");
}

TEST_F(AliasManagerTest, HandlesSpecialCharactersInCommand) {
    am().set("special", "echo 'hello $WORLD' | grep -E \"[a-z]+\"");
    EXPECT_EQ(am().get("special"), "echo 'hello $WORLD' | grep -E \"[a-z]+\"");
}

TEST_F(AliasManagerTest, HandlesQuotedStrings) {
    am().set("quoted", "echo \"hello world\"");
    EXPECT_EQ(am().get("quoted"), "echo \"hello world\"");
}

// ============================================================================
// Singleton Tests
// ============================================================================

TEST_F(AliasManagerTest, InstanceReturnsSameObject) {
    auto& first = AliasManager::instance();
    auto& second = AliasManager::instance();
    EXPECT_EQ(&first, &second);
}
