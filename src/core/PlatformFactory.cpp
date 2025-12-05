#include "core/PlatformFactory.hpp"

#ifdef PLATFORM_WINDOWS
#include "platform/windows/WindowsTerminal.hpp"
#include "platform/windows/WindowsProcessManager.hpp"
#elif defined(PLATFORM_UNIX)
#include "platform/linux/LinuxTerminal.hpp"
#include "platform/linux/LinuxProcessManager.hpp"
#endif

namespace termidash {

std::unique_ptr<platform::ITerminal> createTerminal() {
#ifdef PLATFORM_WINDOWS
    return std::make_unique<platform::windows::WindowsTerminal>();
#elif defined(PLATFORM_UNIX)
    return std::make_unique<platform::linux_platform::LinuxTerminal>();
#else
    return nullptr;
#endif
}

std::unique_ptr<platform::IProcessManager> createProcessManager() {
#ifdef PLATFORM_WINDOWS
    return std::make_unique<platform::windows::WindowsProcessManager>();
#elif defined(PLATFORM_UNIX)
    return std::make_unique<platform::linux_platform::LinuxProcessManager>();
#else
    return nullptr;
#endif
}

} // namespace termidash
