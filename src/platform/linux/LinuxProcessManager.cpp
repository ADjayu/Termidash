#include "platform/linux/LinuxProcessManager.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <vector>
#include <iostream>

namespace termidash {
namespace platform {
namespace linux_platform {

long LinuxProcessManager::spawn(const std::string& command, const std::vector<std::string>& args, bool background,
                                long stdIn, long stdOut, long stdErr) {
    pid_t pid = fork();
    if (pid == -1) {
        lastError = "Fork failed";
        return -1;
    } else if (pid == 0) {
        // Child process
        if (stdIn != -1) {
            dup2((int)stdIn, STDIN_FILENO);
            close((int)stdIn);
        }
        if (stdOut != -1) {
            dup2((int)stdOut, STDOUT_FILENO);
            close((int)stdOut);
        }
        if (stdErr != -1) {
            dup2((int)stdErr, STDERR_FILENO);
            close((int)stdErr);
        }

        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(command.c_str()));
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execvp(command.c_str(), c_args.data());
        // If execvp returns, it failed
        std::cerr << "Exec failed: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        // Parent process
        if (stdIn != -1) close((int)stdIn);
        if (stdOut != -1) close((int)stdOut);
        if (stdErr != -1) close((int)stdErr);

        if (!background) {
            // Wait handled by caller or here?
            // Interface says 'spawn', caller usually waits.
            // But for consistency with Windows implementation where we returned the handle/pid
            return (long)pid;
        }
        return (long)pid;
    }
}

bool LinuxProcessManager::createPipe(long& readHandle, long& writeHandle) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        lastError = "Pipe failed";
        return false;
    }
    readHandle = (long)pipefd[0];
    writeHandle = (long)pipefd[1];
    return true;
}

void LinuxProcessManager::closeHandle(long handle) {
    if (handle != -1) {
        close((int)handle);
    }
}

int LinuxProcessManager::wait(long pid) {
    int status;
    if (waitpid((pid_t)pid, &status, 0) == -1) {
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

bool LinuxProcessManager::kill(long pid) {
    return ::kill((pid_t)pid, SIGTERM) == 0;
}

std::string LinuxProcessManager::getLastError() {
    return lastError;
}

} // namespace linux_platform
} // namespace platform
} // namespace termidash
