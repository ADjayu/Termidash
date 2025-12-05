/**
 * @file test_process_error.cpp
 * @brief Unit tests for ProcessError types
 */

#include <gtest/gtest.h>
#include "platform/interfaces/ProcessError.hpp"

using namespace termidash::platform;

// ============================================================================
// ProcessResult Tests
// ============================================================================

TEST(ProcessResultTest, SuccessResult) {
    auto result = ProcessResult::ok(12345);
    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.pid, 12345);
    EXPECT_EQ(result.error, ProcessError::None);
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST(ProcessResultTest, FailureResult) {
    auto result = ProcessResult::fail(ProcessError::SpawnFailed, "Command not found");
    EXPECT_FALSE(result.success());
    EXPECT_EQ(result.pid, -1);
    EXPECT_EQ(result.error, ProcessError::SpawnFailed);
    EXPECT_EQ(result.errorMessage, "Command not found");
}

TEST(ProcessResultTest, FailureNoMessage) {
    auto result = ProcessResult::fail(ProcessError::NotFound);
    EXPECT_FALSE(result.success());
    EXPECT_EQ(result.error, ProcessError::NotFound);
    EXPECT_TRUE(result.errorMessage.empty());
}

// ============================================================================
// PipeResult Tests
// ============================================================================

TEST(PipeResultTest, SuccessResult) {
    auto result = PipeResult::ok(100, 200);
    EXPECT_TRUE(result.success());
    EXPECT_EQ(result.readHandle, 100);
    EXPECT_EQ(result.writeHandle, 200);
    EXPECT_EQ(result.error, ProcessError::None);
}

TEST(PipeResultTest, FailureResult) {
    auto result = PipeResult::fail(ProcessError::PipeFailed, "Too many open files");
    EXPECT_FALSE(result.success());
    EXPECT_EQ(result.readHandle, -1);
    EXPECT_EQ(result.writeHandle, -1);
    EXPECT_EQ(result.error, ProcessError::PipeFailed);
}

// ============================================================================
// ProcessError toString Tests
// ============================================================================

TEST(ProcessErrorTest, ToStringNone) {
    EXPECT_EQ(toString(ProcessError::None), "None");
}

TEST(ProcessErrorTest, ToStringSpawnFailed) {
    EXPECT_EQ(toString(ProcessError::SpawnFailed), "SpawnFailed");
}

TEST(ProcessErrorTest, ToStringPipeFailed) {
    EXPECT_EQ(toString(ProcessError::PipeFailed), "PipeFailed");
}

TEST(ProcessErrorTest, ToStringNotFound) {
    EXPECT_EQ(toString(ProcessError::NotFound), "NotFound");
}

TEST(ProcessErrorTest, ToStringPermissionDenied) {
    EXPECT_EQ(toString(ProcessError::PermissionDenied), "PermissionDenied");
}

TEST(ProcessErrorTest, ToStringAllErrors) {
    EXPECT_EQ(toString(ProcessError::WaitFailed), "WaitFailed");
    EXPECT_EQ(toString(ProcessError::KillFailed), "KillFailed");
    EXPECT_EQ(toString(ProcessError::InvalidArgument), "InvalidArgument");
    EXPECT_EQ(toString(ProcessError::ResourceLimit), "ResourceLimit");
    EXPECT_EQ(toString(ProcessError::Timeout), "Timeout");
    EXPECT_EQ(toString(ProcessError::Unknown), "Unknown");
}
