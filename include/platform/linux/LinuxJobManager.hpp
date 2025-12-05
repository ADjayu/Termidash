#pragma once

#include "core/JobManager.hpp"
#include <unordered_map>
#include <sys/types.h>
#include <termios.h>

namespace termidash {
    namespace platform {
        namespace linux_platform {

            class LinuxJobManager : public IJobManager {
            public:
                LinuxJobManager();
                ~LinuxJobManager();

                int startJob(const std::string& command) override;
                bool stopJob(int jobId) override;
                bool bringToForeground(int jobId) override;
                bool continueInBackground(int jobId) override;
                std::vector<TermiDashJobInfo> listJobs() override;

            private:
                struct Job {
                    int jobId;
                    std::string command;
                    pid_t pid;
                    bool running;
                    bool background;
                };

                int nextJobId = 1;
                std::unordered_map<int, Job> jobs;
                struct termios shell_tmodes;
            };

        }
    }
}
