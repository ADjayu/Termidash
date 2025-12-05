#include "core/JobManager.hpp"
#include "core/JobManagerFactory.hpp"

#ifdef PLATFORM_WINDOWS
#include "platform/windows/WindowsJobManager.hpp"
#else
#include "platform/linux/LinuxJobManager.hpp"
#endif

std::unique_ptr<IJobManager> createJobManager() {
#ifdef PLATFORM_WINDOWS
    return std::make_unique<WindowsJobManager>();
#else
    return std::make_unique<termidash::platform::linux_platform::LinuxJobManager>();
#endif
}
