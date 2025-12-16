#include "common/SecurityUtils.hpp"
#include <gtest/gtest.h>


using namespace termidash::security;

// ============================================================================
// sanitizeInput Tests
// ============================================================================

TEST(SecurityUtilsTest, SanitizeInput_NormalText) {
  EXPECT_EQ(sanitizeInput("hello world"), "hello world");
}

TEST(SecurityUtilsTest, SanitizeInput_WithNewlines) {
  EXPECT_EQ(sanitizeInput("line1\nline2\r\n"), "line1\nline2\r\n");
}

TEST(SecurityUtilsTest, SanitizeInput_WithTabs) {
  EXPECT_EQ(sanitizeInput("col1\tcol2"), "col1\tcol2");
}

TEST(SecurityUtilsTest, SanitizeInput_RemovesControlChars) {
  std::string input = "hello\x01\x02world"; // Contains control chars
  std::string result = sanitizeInput(input);
  EXPECT_EQ(result, "helloworld");
}

TEST(SecurityUtilsTest, SanitizeInput_EmptyString) {
  EXPECT_EQ(sanitizeInput(""), "");
}

// ============================================================================
// isPathSafe Tests
// ============================================================================

TEST(SecurityUtilsTest, IsPathSafe_NormalPath) {
  EXPECT_TRUE(isPathSafe("myfile.txt"));
  EXPECT_TRUE(isPathSafe("folder/file.txt"));
  EXPECT_TRUE(isPathSafe("C:\\Users\\test\\file.txt"));
}

TEST(SecurityUtilsTest, IsPathSafe_DirectoryTraversal) {
  EXPECT_FALSE(isPathSafe("../secret.txt"));
  EXPECT_FALSE(isPathSafe("folder/../../../etc/passwd"));
  EXPECT_FALSE(isPathSafe("..\\..\\Windows\\System32"));
}

TEST(SecurityUtilsTest, IsPathSafe_SystemPaths) {
  EXPECT_FALSE(isPathSafe("C:\\Windows\\System32\\config"));
  EXPECT_FALSE(isPathSafe("/etc/passwd"));
  EXPECT_FALSE(isPathSafe("/usr/bin/bash"));
}

// ============================================================================
// maskSensitiveArgs Tests
// ============================================================================

TEST(SecurityUtilsTest, MaskSensitiveArgs_Password) {
  std::string masked = maskSensitiveArgs("login --password=secret123");
  EXPECT_TRUE(masked.find("***") != std::string::npos);
  EXPECT_TRUE(masked.find("secret123") == std::string::npos);
}

TEST(SecurityUtilsTest, MaskSensitiveArgs_Token) {
  std::string masked = maskSensitiveArgs("api --token=abc123xyz");
  EXPECT_TRUE(masked.find("***") != std::string::npos);
  EXPECT_TRUE(masked.find("abc123xyz") == std::string::npos);
}

TEST(SecurityUtilsTest, MaskSensitiveArgs_NoSensitiveData) {
  std::string cmd = "echo hello world";
  EXPECT_EQ(maskSensitiveArgs(cmd), cmd);
}

TEST(SecurityUtilsTest, MaskSensitiveArgs_Secret) {
  std::string masked = maskSensitiveArgs("config secret=mysecretvalue");
  EXPECT_TRUE(masked.find("***") != std::string::npos);
}

// ============================================================================
// Safe Mode Tests
// ============================================================================

TEST(SecurityUtilsTest, SafeMode_DefaultDisabled) {
  setSafeMode(false); // Reset
  EXPECT_FALSE(isSafeModeEnabled());
}

TEST(SecurityUtilsTest, SafeMode_CanBeEnabled) {
  setSafeMode(true);
  EXPECT_TRUE(isSafeModeEnabled());
  setSafeMode(false); // Reset
}

TEST(SecurityUtilsTest, SafeMode_AllowsNormalCommands) {
  setSafeMode(true);
  EXPECT_TRUE(isCommandAllowedInSafeMode("echo hello"));
  EXPECT_TRUE(isCommandAllowedInSafeMode("ls -la"));
  EXPECT_TRUE(isCommandAllowedInSafeMode("cat file.txt"));
  setSafeMode(false);
}

TEST(SecurityUtilsTest, SafeMode_BlocksDangerousCommands) {
  setSafeMode(true);
  EXPECT_FALSE(isCommandAllowedInSafeMode("rm -rf /"));
  EXPECT_FALSE(isCommandAllowedInSafeMode("del system.dll"));
  EXPECT_FALSE(isCommandAllowedInSafeMode("format C:"));
  EXPECT_FALSE(isCommandAllowedInSafeMode("sudo su"));
  setSafeMode(false);
}

TEST(SecurityUtilsTest, SafeMode_DisabledAllowsEverything) {
  setSafeMode(false);
  EXPECT_TRUE(isCommandAllowedInSafeMode("rm -rf /"));
  EXPECT_TRUE(isCommandAllowedInSafeMode("format C:"));
}

// ============================================================================
// getBlockedCommands Tests
// ============================================================================

TEST(SecurityUtilsTest, GetBlockedCommands_NotEmpty) {
  auto blocked = getBlockedCommands();
  EXPECT_FALSE(blocked.empty());
  EXPECT_TRUE(blocked.size() > 5); // Should have several blocked commands
}
