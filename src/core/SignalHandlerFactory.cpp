#include "core/SignalHandlerFactory.hpp"

#ifdef PLATFORM_WINDOWS
#include "platform/windows/WindowsSignalHandler.hpp"
#else
#include "platform/linux/LinuxSignalHandler.hpp"
#endif

namespace termidash {

std::unique_ptr<ISignalHandler> createSignalHandler() {
#ifdef PLATFORM_WINDOWS
    return std::make_unique<platform::windows::WindowsSignalHandler>();
#else
    return std::make_unique<platform::linux_platform::LinuxSignalHandler>();
#endif
}

} // namespace termidash
