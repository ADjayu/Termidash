cmake_minimum_required(VERSION 3.16)
project(termidash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific logic
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX)
    add_definitions(-DPLATFORM_UNIX)
endif()

# Include directories
include_directories(include)

# Common Sources
set(CORE_SOURCES
    src/core/ShellLoop.cpp
    src/core/BuiltInCommandHandler.cpp
    src/core/CommandExecutorFactory.cpp
    src/core/JobManagerFactory.cpp
    src/core/SignalHandlerFactory.cpp
    src/core/PlatformFactory.cpp
    src/core/Environment.cpp
    src/core/BuiltIn/CommonCommandHandler.cpp
    src/common/PlatformUtils.cpp
    src/core/AliasManager.cpp
    src/core/VariableManager.cpp
    src/core/FunctionManager.cpp
    src/core/ExpressionEvaluator.cpp
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
    )
endif()

add_executable(termidash src/main.cpp ${CORE_SOURCES} ${PLATFORM_SOURCES})


