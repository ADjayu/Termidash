#include "core/CommandExecutorFactory.hpp"
#include "core/CommandExecutor.hpp"

#ifdef PLATFORM_WINDOWS
#include "platform/windows/WindowsCommandExecutor.hpp"
#endif

std::unique_ptr<ICommandExecutor> createCommandExecutor() {
#ifdef PLATFORM_WINDOWS
    return std::make_unique<WindowsCommandExecutor>();
#else
    return nullptr;
#endif
}
