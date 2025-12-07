#include <gtest/gtest.h>
#include "core/GlobExpander.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace termidash;

// Test fixture that creates temporary test files
class GlobExpanderTest : public ::testing::Test {
protected:
    std::string testDir;
    
    void SetUp() override {
        // Create a unique test directory
        testDir = (fs::temp_directory_path() / "glob_test_XXXXXX").string();
        fs::create_directories(testDir);
        
        // Create test files
        createFile(testDir + "/file1.txt");
        createFile(testDir + "/file2.txt");
        createFile(testDir + "/file3.log");
        createFile(testDir + "/test_a.txt");
        createFile(testDir + "/test_b.txt");
        createFile(testDir + "/.hidden");
        
        // Create subdirectory with files
        fs::create_directories(testDir + "/subdir");
        createFile(testDir + "/subdir/nested.txt");
        createFile(testDir + "/subdir/other.log");
    }
    
    void TearDown() override {
        // Clean up test directory
        try {
            fs::remove_all(testDir);
        } catch (...) {}
    }
    
    void createFile(const std::string& path) {
        std::ofstream(path).close();
    }
};

// ============================================================================
// hasGlobChars Tests
// ============================================================================

TEST(GlobExpander, HasGlobChars_True) {
    EXPECT_TRUE(GlobExpander::hasGlobChars("*.txt"));
    EXPECT_TRUE(GlobExpander::hasGlobChars("file?.log"));
    EXPECT_TRUE(GlobExpander::hasGlobChars("[abc].txt"));
    EXPECT_TRUE(GlobExpander::hasGlobChars("**/*.cpp"));
}

TEST(GlobExpander, HasGlobChars_False) {
    EXPECT_FALSE(GlobExpander::hasGlobChars("plain.txt"));
    EXPECT_FALSE(GlobExpander::hasGlobChars("path/to/file"));
    EXPECT_FALSE(GlobExpander::hasGlobChars("no_globs_here"));
}

// ============================================================================
// Pattern Matching Tests (Internal Logic)
// ============================================================================

TEST(GlobExpander, NoGlob_ReturnsOriginal) {
    auto result = GlobExpander::expand("nopattern.txt");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "nopattern.txt");
}

// ============================================================================
// File System Tests
// ============================================================================

TEST_F(GlobExpanderTest, StarPattern) {
    auto result = GlobExpander::expand(testDir + "/*.txt");
    // Should match file1.txt, file2.txt, test_a.txt, test_b.txt
    ASSERT_GE(result.size(), 4);
    
    bool hasFile1 = false, hasFile2 = false;
    for (const auto& r : result) {
        if (r.find("file1.txt") != std::string::npos) hasFile1 = true;
        if (r.find("file2.txt") != std::string::npos) hasFile2 = true;
    }
    EXPECT_TRUE(hasFile1);
    EXPECT_TRUE(hasFile2);
}

TEST_F(GlobExpanderTest, QuestionMarkPattern) {
    auto result = GlobExpander::expand(testDir + "/file?.txt");
    // Should match file1.txt, file2.txt
    ASSERT_EQ(result.size(), 2);
}

TEST_F(GlobExpanderTest, CharClassPattern) {
    auto result = GlobExpander::expand(testDir + "/file[12].txt");
    // Should match file1.txt, file2.txt
    ASSERT_EQ(result.size(), 2);
}

TEST_F(GlobExpanderTest, CharRangePattern) {
    auto result = GlobExpander::expand(testDir + "/test_[a-b].txt");
    // Should match test_a.txt, test_b.txt
    ASSERT_EQ(result.size(), 2);
}

TEST_F(GlobExpanderTest, HiddenFilesNotMatchedByDefault) {
    auto result = GlobExpander::expand(testDir + "/*");
    // Should NOT include .hidden by default
    bool hasHidden = false;
    for (const auto& r : result) {
        if (r.find(".hidden") != std::string::npos) hasHidden = true;
    }
    EXPECT_FALSE(hasHidden);
}

TEST_F(GlobExpanderTest, HiddenFilesMatchedWithDotPattern) {
    auto result = GlobExpander::expand(testDir + "/.*");
    // Should match .hidden
    bool hasHidden = false;
    for (const auto& r : result) {
        if (r.find(".hidden") != std::string::npos) hasHidden = true;
    }
    EXPECT_TRUE(hasHidden);
}

TEST_F(GlobExpanderTest, NoMatchReturnsOriginal) {
    auto result = GlobExpander::expand(testDir + "/nonexistent*.xyz");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], testDir + "/nonexistent*.xyz");
}

TEST_F(GlobExpanderTest, SubdirectoryPattern) {
    auto result = GlobExpander::expand(testDir + "/subdir/*.txt");
    // Should match subdir/nested.txt
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(result[0].find("nested.txt") != std::string::npos);
}

// ============================================================================
// ExpandTokens Tests
// ============================================================================

TEST_F(GlobExpanderTest, ExpandTokens) {
    std::vector<std::string> tokens = {
        testDir + "/file1.txt",  // No glob
        testDir + "/*.log"       // With glob
    };
    auto result = GlobExpander::expandTokens(tokens);
    
    // First token unchanged, second expands to file3.log
    ASSERT_GE(result.size(), 2);
    EXPECT_TRUE(result[0].find("file1.txt") != std::string::npos);
}
