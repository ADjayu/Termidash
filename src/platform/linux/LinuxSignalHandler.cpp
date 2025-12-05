#include "platform/linux/LinuxSignalHandler.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

namespace termidash {
namespace platform {
namespace linux_platform {

void LinuxSignalHandler::setupHandlers() {
    struct sigaction sa;
    sa.sa_handler = &LinuxSignalHandler::handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "Error: Could not set SIGINT handler" << std::endl;
    }
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        std::cerr << "Error: Could not set SIGTSTP handler" << std::endl;
    }
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        std::cerr << "Error: Could not set SIGCHLD handler" << std::endl;
    }
}

void LinuxSignalHandler::resetHandlers() {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}

void LinuxSignalHandler::handleSignal(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n^C" << std::endl;
    } else if (signal == SIGTSTP) {
        std::cout << "\n^Z" << std::endl;
    } else if (signal == SIGCHLD) {
        // Reap zombie processes
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }
}

} // namespace linux_platform
} // namespace platform
} // namespace termidash
