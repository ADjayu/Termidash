#include <gtest/gtest.h>

// Main entry point for GoogleTest
// This file exists to allow tests to be split across multiple files
// while still having a single main function

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
