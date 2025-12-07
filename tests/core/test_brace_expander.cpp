#include <gtest/gtest.h>
#include "core/BraceExpander.hpp"

using namespace termidash;

// ============================================================================
// hasBraces Tests
// ============================================================================

TEST(BraceExpander, HasBraces_True) {
    EXPECT_TRUE(BraceExpander::hasBraces("{a,b,c}"));
    EXPECT_TRUE(BraceExpander::hasBraces("file{1,2}.txt"));
    EXPECT_TRUE(BraceExpander::hasBraces("{1..5}"));
    EXPECT_TRUE(BraceExpander::hasBraces("prefix{a,b}suffix"));
}

TEST(BraceExpander, HasBraces_False) {
    EXPECT_FALSE(BraceExpander::hasBraces("plain text"));
    EXPECT_FALSE(BraceExpander::hasBraces("no braces"));
    EXPECT_FALSE(BraceExpander::hasBraces("{unmatched"));
    EXPECT_FALSE(BraceExpander::hasBraces("unmatched}"));
}

// ============================================================================
// Comma List Expansion Tests
// ============================================================================

TEST(BraceExpander, Expand_SimpleCommaList) {
    auto result = BraceExpander::expand("{a,b,c}");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST(BraceExpander, Expand_WithPrefix) {
    auto result = BraceExpander::expand("file{1,2,3}");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "file1");
    EXPECT_EQ(result[1], "file2");
    EXPECT_EQ(result[2], "file3");
}

TEST(BraceExpander, Expand_WithSuffix) {
    auto result = BraceExpander::expand("{a,b}.txt");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "a.txt");
    EXPECT_EQ(result[1], "b.txt");
}

TEST(BraceExpander, Expand_WithPrefixAndSuffix) {
    auto result = BraceExpander::expand("file{1,2}.txt");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "file1.txt");
    EXPECT_EQ(result[1], "file2.txt");
}

// ============================================================================
// Numeric Range Tests
// ============================================================================

TEST(BraceExpander, Expand_NumericRange) {
    auto result = BraceExpander::expand("{1..5}");
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], "1");
    EXPECT_EQ(result[1], "2");
    EXPECT_EQ(result[2], "3");
    EXPECT_EQ(result[3], "4");
    EXPECT_EQ(result[4], "5");
}

TEST(BraceExpander, Expand_NumericRangeWithPrefix) {
    auto result = BraceExpander::expand("file{1..3}");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "file1");
    EXPECT_EQ(result[1], "file2");
    EXPECT_EQ(result[2], "file3");
}

TEST(BraceExpander, Expand_NumericRangeReverse) {
    auto result = BraceExpander::expand("{5..1}");
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], "5");
    EXPECT_EQ(result[1], "4");
    EXPECT_EQ(result[2], "3");
    EXPECT_EQ(result[3], "2");
    EXPECT_EQ(result[4], "1");
}

// ============================================================================
// Character Range Tests
// ============================================================================

TEST(BraceExpander, Expand_CharRange) {
    auto result = BraceExpander::expand("{a..e}");
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
    EXPECT_EQ(result[3], "d");
    EXPECT_EQ(result[4], "e");
}

TEST(BraceExpander, Expand_CharRangeWithContext) {
    auto result = BraceExpander::expand("file_{a..c}.txt");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "file_a.txt");
    EXPECT_EQ(result[1], "file_b.txt");
    EXPECT_EQ(result[2], "file_c.txt");
}

TEST(BraceExpander, Expand_CharRangeReverse) {
    auto result = BraceExpander::expand("{z..x}");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "z");
    EXPECT_EQ(result[1], "y");
    EXPECT_EQ(result[2], "x");
}

// ============================================================================
// Nested Brace Tests
// ============================================================================

TEST(BraceExpander, Expand_Nested) {
    auto result = BraceExpander::expand("{a,{b,c}}");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST(BraceExpander, Expand_MultipleSequential) {
    auto result = BraceExpander::expand("{a,b}{1,2}");
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "a1");
    EXPECT_EQ(result[1], "a2");
    EXPECT_EQ(result[2], "b1");
    EXPECT_EQ(result[3], "b2");
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(BraceExpander, Expand_NoBraces) {
    auto result = BraceExpander::expand("plain text");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "plain text");
}

TEST(BraceExpander, Expand_EmptyBraces) {
    auto result = BraceExpander::expand("{}");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "");
}

TEST(BraceExpander, Expand_SingleItem) {
    // Single item in braces (no comma) - treated as literal
    auto result = BraceExpander::expand("{solo}");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "solo");
}

TEST(BraceExpander, Expand_UnmatchedOpen) {
    auto result = BraceExpander::expand("{unmatched");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "{unmatched");
}

TEST(BraceExpander, Expand_SpecialCharacters) {
    auto result = BraceExpander::expand("{file-1,file_2}");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "file-1");
    EXPECT_EQ(result[1], "file_2");
}
