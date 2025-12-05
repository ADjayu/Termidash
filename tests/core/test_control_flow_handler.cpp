/**
 * @file test_control_flow_handler.cpp
 * @brief Unit tests for the ControlFlowHandler class
 */

#include <gtest/gtest.h>
#include "core/ControlFlowHandler.hpp"

using namespace termidash;

// ============================================================================
// startsBlock Tests
// ============================================================================

TEST(ControlFlowHandlerTest, StartsBlockIf) {
    EXPECT_TRUE(ControlFlowHandler::startsBlock("if true"));
    EXPECT_TRUE(ControlFlowHandler::startsBlock("if test -f file.txt"));
}

TEST(ControlFlowHandlerTest, StartsBlockWhile) {
    EXPECT_TRUE(ControlFlowHandler::startsBlock("while true"));
    EXPECT_TRUE(ControlFlowHandler::startsBlock("while [ $x -gt 0 ]"));
}

TEST(ControlFlowHandlerTest, StartsBlockFor) {
    EXPECT_TRUE(ControlFlowHandler::startsBlock("for i in 1 2 3"));
    EXPECT_TRUE(ControlFlowHandler::startsBlock("for file in *.txt"));
}

TEST(ControlFlowHandlerTest, StartsBlockFunction) {
    EXPECT_TRUE(ControlFlowHandler::startsBlock("function myFunc"));
    EXPECT_TRUE(ControlFlowHandler::startsBlock("myFunc() {"));
}

TEST(ControlFlowHandlerTest, DoesNotStartBlock) {
    EXPECT_FALSE(ControlFlowHandler::startsBlock("echo hello"));
    EXPECT_FALSE(ControlFlowHandler::startsBlock("ls -la"));
    EXPECT_FALSE(ControlFlowHandler::startsBlock(""));
}

// ============================================================================
// endsBlock Tests
// ============================================================================

TEST(ControlFlowHandlerTest, EndsBlockEnd) {
    EXPECT_TRUE(ControlFlowHandler::endsBlock("end"));
}

TEST(ControlFlowHandlerTest, EndsBlockBrace) {
    EXPECT_TRUE(ControlFlowHandler::endsBlock("}"));
}

TEST(ControlFlowHandlerTest, DoesNotEndBlock) {
    EXPECT_FALSE(ControlFlowHandler::endsBlock("endif"));
    EXPECT_FALSE(ControlFlowHandler::endsBlock("done"));
    EXPECT_FALSE(ControlFlowHandler::endsBlock("echo end"));
}

// ============================================================================
// isElse Tests
// ============================================================================

TEST(ControlFlowHandlerTest, IsElse) {
    EXPECT_TRUE(ControlFlowHandler::isElse("else"));
}

TEST(ControlFlowHandlerTest, IsNotElse) {
    EXPECT_FALSE(ControlFlowHandler::isElse("elif"));
    EXPECT_FALSE(ControlFlowHandler::isElse("else if"));
    EXPECT_FALSE(ControlFlowHandler::isElse(""));
}

// ============================================================================
// parseIf Tests
// ============================================================================

TEST(ControlFlowHandlerTest, ParseIf) {
    Block b = ControlFlowHandler::parseIf("if test condition");
    EXPECT_EQ(b.type, BlockType::If);
    EXPECT_EQ(b.condition, "test condition");
}

TEST(ControlFlowHandlerTest, ParseIfSimple) {
    Block b = ControlFlowHandler::parseIf("if true");
    EXPECT_EQ(b.type, BlockType::If);
    EXPECT_EQ(b.condition, "true");
}

// ============================================================================
// parseWhile Tests
// ============================================================================

TEST(ControlFlowHandlerTest, ParseWhile) {
    Block b = ControlFlowHandler::parseWhile("while test -f file");
    EXPECT_EQ(b.type, BlockType::While);
    EXPECT_EQ(b.condition, "test -f file");
}

// ============================================================================
// parseFor Tests
// ============================================================================

TEST(ControlFlowHandlerTest, ParseForSimple) {
    Block b = ControlFlowHandler::parseFor("for i in 1 2 3");
    EXPECT_EQ(b.type, BlockType::For);
    EXPECT_EQ(b.loopVar, "i");
    EXPECT_EQ(b.items.size(), 3);
    EXPECT_EQ(b.items[0], "1");
    EXPECT_EQ(b.items[1], "2");
    EXPECT_EQ(b.items[2], "3");
}

TEST(ControlFlowHandlerTest, ParseForFiles) {
    Block b = ControlFlowHandler::parseFor("for file in a.txt b.txt c.txt");
    EXPECT_EQ(b.type, BlockType::For);
    EXPECT_EQ(b.loopVar, "file");
    EXPECT_EQ(b.items.size(), 3);
}

TEST(ControlFlowHandlerTest, ParseForNoIn) {
    // Invalid for loop without "in"
    Block b = ControlFlowHandler::parseFor("for i 1 2 3");
    EXPECT_EQ(b.type, BlockType::For);
    // loopVar and items should be empty since no "in" found
    EXPECT_TRUE(b.loopVar.empty());
    EXPECT_TRUE(b.items.empty());
}

// ============================================================================
// parseFunction Tests
// ============================================================================

TEST(ControlFlowHandlerTest, ParseFunctionKeyword) {
    Block b = ControlFlowHandler::parseFunction("function myFunc");
    EXPECT_EQ(b.type, BlockType::Function);
    EXPECT_EQ(b.condition, "myFunc");
}

TEST(ControlFlowHandlerTest, ParseFunctionParens) {
    Block b = ControlFlowHandler::parseFunction("myFunc() {");
    EXPECT_EQ(b.type, BlockType::Function);
    EXPECT_EQ(b.condition, "myFunc");
}

TEST(ControlFlowHandlerTest, ParseFunctionWithBrace) {
    Block b = ControlFlowHandler::parseFunction("function myFunc {");
    EXPECT_EQ(b.type, BlockType::Function);
    EXPECT_EQ(b.condition, "myFunc");
}

// ============================================================================
// Block State Tests
// ============================================================================

TEST(ControlFlowHandlerTest, ShellStateEmpty) {
    ShellState state;
    EXPECT_FALSE(state.inBlock());
}

TEST(ControlFlowHandlerTest, ShellStateInBlock) {
    ShellState state;
    Block b;
    b.type = BlockType::If;
    state.blockStack.push_back(b);
    EXPECT_TRUE(state.inBlock());
}

TEST(ControlFlowHandlerTest, BlockBodyManagement) {
    Block b;
    b.type = BlockType::If;
    b.body.push_back("echo line1");
    b.body.push_back("echo line2");
    EXPECT_EQ(b.body.size(), 2);
    EXPECT_FALSE(b.inElse);
}

TEST(ControlFlowHandlerTest, BlockElseBody) {
    Block b;
    b.type = BlockType::If;
    b.inElse = true;
    b.elseBody.push_back("echo else1");
    EXPECT_TRUE(b.inElse);
    EXPECT_EQ(b.elseBody.size(), 1);
}
