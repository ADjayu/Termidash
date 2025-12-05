#pragma once

#include <string>
#include <vector>

struct TermiDashJobInfo {
    int jobId;
    std::string command;
    unsigned long pid;
    std::string status;
};

class IJobManager {
public:
    virtual int startJob(const std::string& command) = 0;
    virtual bool stopJob(int jobId) = 0;
    virtual bool bringToForeground(int jobId) = 0;
    virtual bool continueInBackground(int jobId) = 0;
    virtual std::vector<TermiDashJobInfo> listJobs() = 0;
    virtual ~IJobManager() {}
};
