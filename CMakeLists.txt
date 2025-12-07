cmake_minimum_required(VERSION 3.16)
project(termidash VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTING "Build the testing tree" ON)

# Platform-specific logic
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX)
    add_definitions(-DPLATFORM_UNIX)
endif()

# Include directories
include_directories(include)

# Testable Core Sources (non-platform specific, no main)
set(CORE_TESTABLE_SOURCES
    src/core/AliasManager.cpp
    src/core/VariableManager.cpp
    src/core/FunctionManager.cpp
    src/core/ExpressionEvaluator.cpp
    src/core/Environment.cpp
    src/core/Parser.cpp
    src/core/CompletionEngine.cpp
    src/core/InputHandler.cpp
    src/core/ControlFlowHandler.cpp
    src/core/CommandSubstitution.cpp
    src/core/BraceExpander.cpp
    src/core/GlobExpander.cpp
    src/core/PromptEngine.cpp
)

# Core Sources that need platform
set(CORE_SOURCES
    src/core/ShellLoop.cpp
    src/core/BuiltInCommandHandler.cpp
    src/core/CommandExecutorFactory.cpp
    src/core/JobManagerFactory.cpp
    src/core/SignalHandlerFactory.cpp
    src/core/PlatformFactory.cpp
    src/core/BuiltIn/CommonCommandHandler.cpp
    src/common/PlatformUtils.cpp
    src/core/PipelineExecutor.cpp
)

# Platform Sources
if(WIN32)
    set(PLATFORM_SOURCES
        src/platform/windows/WindowsCommandExecutor.cpp
        src/platform/windows/WindowsJobManager.cpp
        src/platform/windows/WindowsTerminal.cpp
        src/platform/windows/WindowsProcessManager.cpp
        src/platform/windows/WindowsSignalHandler.cpp
        src/core/BuiltIn/WindowsCommandHandler.cpp
    )
else() # Assuming UNIX for non-WIN32
    set(PLATFORM_SOURCES
        src/platform/linux/LinuxTerminal.cpp
        src/platform/linux/LinuxProcessManager.cpp
        src/platform/linux/LinuxJobManager.cpp
        src/platform/linux/LinuxSignalHandler.cpp
        src/core/BuiltIn/LinuxCommandHandler.cpp
    )
endif()

# Create a library for testable components
add_library(termidash_core STATIC ${CORE_TESTABLE_SOURCES})
target_include_directories(termidash_core PUBLIC include)

# Main executable
add_executable(termidash src/main.cpp ${CORE_SOURCES} ${PLATFORM_SOURCES})
target_link_libraries(termidash PRIVATE termidash_core)

# ============================================================================
# Testing Configuration
# ============================================================================
if(BUILD_TESTING)
    enable_testing()
    
    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Test sources
    set(TEST_SOURCES
        tests/test_main.cpp
        tests/core/test_expression_evaluator.cpp
        tests/core/test_variable_manager.cpp
        tests/core/test_alias_manager.cpp
        tests/core/test_function_manager.cpp
        tests/core/test_parser.cpp
        tests/core/test_completion_engine.cpp
        tests/core/test_control_flow_handler.cpp
        tests/core/test_process_error.cpp
        tests/core/test_command_substitution.cpp
        tests/core/test_brace_expander.cpp
        tests/core/test_glob_expander.cpp
        tests/core/test_prompt_engine.cpp
    )
    
    # Test executable
    add_executable(termidash_tests ${TEST_SOURCES})
    target_link_libraries(termidash_tests 
        PRIVATE 
            termidash_core
            GTest::gtest
            GTest::gtest_main
    )
    target_include_directories(termidash_tests PRIVATE include)
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(termidash_tests)
endif()


