#include "platform/windows/WindowsJobManager.hpp"
#include <sstream>
#include <iostream>

int WindowsJobManager::startJob(const std::string& command) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    std::string cmdLine = "cmd.exe /C " + command;
    char* cmd = _strdup(cmdLine.c_str());

    BOOL success = CreateProcessA(
        nullptr,
        cmd,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    free(cmd);

    if (!success) {
        std::cerr << "Failed to start job. Error code: " << GetLastError() << std::endl;
        return -1;
    }

    WindowsJob job;
    job.jobId = nextJobId++;
    job.command = command;
    job.processHandle = pi.hProcess;
    job.pid = pi.dwProcessId;

    jobs[job.jobId] = job;

    CloseHandle(pi.hThread); // only need process handle

    std::cout << "[Job " << job.jobId << "] started in background with PID " << job.pid << std::endl;

    return job.jobId;
}

bool WindowsJobManager::stopJob(int jobId) {
    if (jobs.find(jobId) == jobs.end()) return false;

    BOOL success = TerminateProcess(jobs[jobId].processHandle, 1);
    CloseHandle(jobs[jobId].processHandle);
    jobs.erase(jobId);
    return success == TRUE;
}

bool WindowsJobManager::bringToForeground(int jobId) {
    // Not fully implemented on Windows yet
    if (jobs.find(jobId) == jobs.end()) return false;
    std::cout << "Bringing job " << jobId << " to foreground (simulated wait)" << std::endl;
    WaitForSingleObject(jobs[jobId].processHandle, INFINITE);
    return true;
}

bool WindowsJobManager::continueInBackground(int jobId) {
    // Not fully implemented on Windows yet
    if (jobs.find(jobId) == jobs.end()) return false;
    std::cout << "Continuing job " << jobId << " in background" << std::endl;
    return true;
}

std::vector<TermiDashJobInfo> WindowsJobManager::listJobs() {
    std::vector<TermiDashJobInfo> jobList;
    for (const auto& [id, job] : jobs) {
        DWORD exitCode = 0;
        GetExitCodeProcess(job.processHandle, &exitCode);

        TermiDashJobInfo info;
        info.jobId = job.jobId;
        info.command = job.command;
        info.pid = job.pid;
        info.status = (exitCode == STILL_ACTIVE) ? "Running" : "Exited";

        jobList.push_back(info);
    }
    return jobList;
}
