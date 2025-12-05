/**
 * @file test_function_manager.cpp
 * @brief Unit tests for the FunctionManager class
 */

#include <gtest/gtest.h>
#include "core/FunctionManager.hpp"

using namespace termidash;

class FunctionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear any functions from previous tests
        auto& fm = FunctionManager::instance();
        auto all = fm.getAll();
        for (const auto& pair : all) {
            fm.unset(pair.first);
        }
    }
    
    FunctionManager& fm() {
        return FunctionManager::instance();
    }
};

// ============================================================================
// Basic Define/Get Tests
// ============================================================================

TEST_F(FunctionManagerTest, CanDefineAndGetFunction) {
    std::vector<std::string> body = {"echo hello", "echo world"};
    fm().define("greet", body);
    
    EXPECT_TRUE(fm().has("greet"));
    const auto& retrieved = fm().getBody("greet");
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved[0], "echo hello");
    EXPECT_EQ(retrieved[1], "echo world");
}

TEST_F(FunctionManagerTest, HasReturnsTrueForExistingFunction) {
    std::vector<std::string> body = {"echo test"};
    fm().define("exists", body);
    EXPECT_TRUE(fm().has("exists"));
}

TEST_F(FunctionManagerTest, HasReturnsFalseForNonExistingFunction) {
    EXPECT_FALSE(fm().has("does_not_exist"));
}

TEST_F(FunctionManagerTest, CanOverwriteFunction) {
    std::vector<std::string> body1 = {"echo first"};
    std::vector<std::string> body2 = {"echo second", "echo line2"};
    
    fm().define("func", body1);
    fm().define("func", body2);
    
    const auto& retrieved = fm().getBody("func");
    EXPECT_EQ(retrieved.size(), 2);
    EXPECT_EQ(retrieved[0], "echo second");
}

// ============================================================================
// Unset Tests
// ============================================================================

TEST_F(FunctionManagerTest, UnsetRemovesFunction) {
    std::vector<std::string> body = {"echo test"};
    fm().define("to_remove", body);
    EXPECT_TRUE(fm().has("to_remove"));
    
    fm().unset("to_remove");
    EXPECT_FALSE(fm().has("to_remove"));
}

TEST_F(FunctionManagerTest, UnsetNonexistentDoesNotThrow) {
    EXPECT_NO_THROW(fm().unset("never_defined"));
}

// ============================================================================
// GetAll Tests
// ============================================================================

TEST_F(FunctionManagerTest, GetAllReturnsAllFunctions) {
    std::vector<std::string> body1 = {"cmd1"};
    std::vector<std::string> body2 = {"cmd2"};
    std::vector<std::string> body3 = {"cmd3"};
    
    fm().define("func1", body1);
    fm().define("func2", body2);
    fm().define("func3", body3);
    
    auto all = fm().getAll();
    EXPECT_EQ(all.size(), 3);
    EXPECT_TRUE(all.find("func1") != all.end());
    EXPECT_TRUE(all.find("func2") != all.end());
    EXPECT_TRUE(all.find("func3") != all.end());
}

TEST_F(FunctionManagerTest, GetAllReturnsEmptyWhenNoFunctions) {
    auto all = fm().getAll();
    EXPECT_TRUE(all.empty());
}

// ============================================================================
// Typical Function Bodies
// ============================================================================

TEST_F(FunctionManagerTest, HandlesMultiLineFunction) {
    std::vector<std::string> body = {
        "echo 'Starting backup...'",
        "cp -r /source /dest",
        "echo 'Backup complete!'"
    };
    fm().define("backup", body);
    
    const auto& retrieved = fm().getBody("backup");
    EXPECT_EQ(retrieved.size(), 3);
}

TEST_F(FunctionManagerTest, HandlesFunctionWithVariables) {
    std::vector<std::string> body = {
        "echo \"Hello, $1\"",
        "echo \"Your age is $2\""
    };
    fm().define("greet_user", body);
    
    const auto& retrieved = fm().getBody("greet_user");
    EXPECT_EQ(retrieved[0], "echo \"Hello, $1\"");
    EXPECT_EQ(retrieved[1], "echo \"Your age is $2\"");
}

TEST_F(FunctionManagerTest, HandlesFunctionWithControlFlow) {
    std::vector<std::string> body = {
        "if test -f $1",
        "  echo 'File exists'",
        "else",
        "  echo 'File does not exist'",
        "end"
    };
    fm().define("check_file", body);
    
    const auto& retrieved = fm().getBody("check_file");
    EXPECT_EQ(retrieved.size(), 5);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FunctionManagerTest, HandlesEmptyFunctionName) {
    std::vector<std::string> body = {"echo test"};
    fm().define("", body);
    EXPECT_TRUE(fm().has(""));
}

TEST_F(FunctionManagerTest, HandlesEmptyBody) {
    std::vector<std::string> body;
    fm().define("empty_func", body);
    
    EXPECT_TRUE(fm().has("empty_func"));
    const auto& retrieved = fm().getBody("empty_func");
    EXPECT_TRUE(retrieved.empty());
}

TEST_F(FunctionManagerTest, HandlesSingleLineFunction) {
    std::vector<std::string> body = {"echo 'one liner'"};
    fm().define("oneliner", body);
    
    const auto& retrieved = fm().getBody("oneliner");
    EXPECT_EQ(retrieved.size(), 1);
}

TEST_F(FunctionManagerTest, HandlesSpecialCharactersInBody) {
    std::vector<std::string> body = {
        "echo 'Special chars: $var @at #hash'",
        "grep -E '[a-z]+' file.txt | sort | uniq"
    };
    fm().define("special", body);
    
    const auto& retrieved = fm().getBody("special");
    EXPECT_EQ(retrieved[0], "echo 'Special chars: $var @at #hash'");
}

// ============================================================================
// Singleton Tests
// ============================================================================

TEST_F(FunctionManagerTest, InstanceReturnsSameObject) {
    auto& first = FunctionManager::instance();
    auto& second = FunctionManager::instance();
    EXPECT_EQ(&first, &second);
}
