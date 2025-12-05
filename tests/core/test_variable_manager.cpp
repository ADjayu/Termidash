/**
 * @file test_variable_manager.cpp
 * @brief Unit tests for the VariableManager class
 */

#include <gtest/gtest.h>
#include "core/VariableManager.hpp"

using namespace termidash;

class VariableManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear any variables from previous tests
        auto& vm = VariableManager::instance();
        auto all = vm.getAll();
        for (const auto& pair : all) {
            vm.unset(pair.first);
        }
    }
    
    VariableManager& vm() {
        return VariableManager::instance();
    }
};

// ============================================================================
// Basic Set/Get Tests
// ============================================================================

TEST_F(VariableManagerTest, CanSetAndGetVariable) {
    vm().set("TEST_VAR", "test_value");
    EXPECT_EQ(vm().get("TEST_VAR"), "test_value");
}

TEST_F(VariableManagerTest, HasReturnsTrueForExistingVariable) {
    vm().set("EXISTS", "value");
    EXPECT_TRUE(vm().has("EXISTS"));
}

TEST_F(VariableManagerTest, HasReturnsFalseForNonExistingVariable) {
    EXPECT_FALSE(vm().has("DOES_NOT_EXIST"));
}

TEST_F(VariableManagerTest, GetReturnsEmptyForNonExistingVariable) {
    EXPECT_EQ(vm().get("NONEXISTENT"), "");
}

TEST_F(VariableManagerTest, CanOverwriteVariable) {
    vm().set("VAR", "first");
    vm().set("VAR", "second");
    EXPECT_EQ(vm().get("VAR"), "second");
}

// ============================================================================
// Unset Tests
// ============================================================================

TEST_F(VariableManagerTest, UnsetRemovesVariable) {
    vm().set("TO_REMOVE", "value");
    EXPECT_TRUE(vm().has("TO_REMOVE"));
    
    vm().unset("TO_REMOVE");
    EXPECT_FALSE(vm().has("TO_REMOVE"));
}

TEST_F(VariableManagerTest, UnsetNonexistentDoesNotThrow) {
    EXPECT_NO_THROW(vm().unset("NEVER_SET"));
}

// ============================================================================
// GetAll Tests
// ============================================================================

TEST_F(VariableManagerTest, GetAllReturnsAllVariables) {
    vm().set("VAR1", "value1");
    vm().set("VAR2", "value2");
    vm().set("VAR3", "value3");
    
    auto all = vm().getAll();
    EXPECT_EQ(all.size(), 3);
    EXPECT_EQ(all["VAR1"], "value1");
    EXPECT_EQ(all["VAR2"], "value2");
    EXPECT_EQ(all["VAR3"], "value3");
}

TEST_F(VariableManagerTest, GetAllReturnsEmptyWhenNoVariables) {
    auto all = vm().getAll();
    EXPECT_TRUE(all.empty());
}

// ============================================================================
// Scope Tests
// ============================================================================

TEST_F(VariableManagerTest, PushScopeCreatesNewScope) {
    vm().set("GLOBAL", "global_value");
    vm().pushScope();
    vm().set("LOCAL", "local_value");
    
    EXPECT_EQ(vm().get("GLOBAL"), "global_value");
    EXPECT_EQ(vm().get("LOCAL"), "local_value");
}

TEST_F(VariableManagerTest, PopScopeRemovesLocalVariables) {
    vm().set("GLOBAL", "global_value");
    vm().pushScope();
    vm().set("LOCAL", "local_value");
    vm().popScope();
    
    EXPECT_EQ(vm().get("GLOBAL"), "global_value");
    EXPECT_FALSE(vm().has("LOCAL"));
}

TEST_F(VariableManagerTest, LocalVariableShadowsGlobal) {
    vm().set("VAR", "global");
    vm().pushScope();
    vm().set("VAR", "local");
    
    EXPECT_EQ(vm().get("VAR"), "local");
    
    vm().popScope();
    EXPECT_EQ(vm().get("VAR"), "global");
}

TEST_F(VariableManagerTest, NestedScopes) {
    vm().set("VAR", "level0");
    vm().pushScope();
    vm().set("VAR", "level1");
    vm().pushScope();
    vm().set("VAR", "level2");
    
    EXPECT_EQ(vm().get("VAR"), "level2");
    
    vm().popScope();
    EXPECT_EQ(vm().get("VAR"), "level1");
    
    vm().popScope();
    EXPECT_EQ(vm().get("VAR"), "level0");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(VariableManagerTest, HandlesEmptyVariableName) {
    vm().set("", "empty_name");
    EXPECT_EQ(vm().get(""), "empty_name");
    EXPECT_TRUE(vm().has(""));
}

TEST_F(VariableManagerTest, HandlesEmptyValue) {
    vm().set("EMPTY_VAL", "");
    EXPECT_TRUE(vm().has("EMPTY_VAL"));
    EXPECT_EQ(vm().get("EMPTY_VAL"), "");
}

TEST_F(VariableManagerTest, HandlesSpecialCharactersInValue) {
    vm().set("SPECIAL", "value with spaces and $special @chars!");
    EXPECT_EQ(vm().get("SPECIAL"), "value with spaces and $special @chars!");
}

TEST_F(VariableManagerTest, HandlesNumericNames) {
    vm().set("123", "numeric_name");
    EXPECT_EQ(vm().get("123"), "numeric_name");
}

// ============================================================================
// Singleton Tests
// ============================================================================

TEST_F(VariableManagerTest, InstanceReturnsSameObject) {
    auto& first = VariableManager::instance();
    auto& second = VariableManager::instance();
    EXPECT_EQ(&first, &second);
}
