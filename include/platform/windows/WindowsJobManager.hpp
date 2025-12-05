#pragma once

#include "core/JobManager.hpp"
#include <unordered_map>
#include <windows.h>
#include <vector>

class WindowsJobManager : public IJobManager {
public:
    int startJob(const std::string& command) override;
    bool stopJob(int jobId) override;
    bool bringToForeground(int jobId) override;
    bool continueInBackground(int jobId) override;
    std::vector<TermiDashJobInfo> listJobs() override;

private:
    struct WindowsJob {
        int jobId;
        std::string command;
        HANDLE processHandle;
        DWORD pid;
    };

    int nextJobId = 1;
    std::unordered_map<int, WindowsJob> jobs;
};
