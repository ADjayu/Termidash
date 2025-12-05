#include "platform/linux/LinuxJobManager.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <sstream>
#include <cstring>

namespace termidash {
    namespace platform {
        namespace linux_platform {

            LinuxJobManager::LinuxJobManager() {
                // Save current shell terminal modes
                tcgetattr(STDIN_FILENO, &shell_tmodes);
            }

            LinuxJobManager::~LinuxJobManager() {
                // Cleanup jobs?
            }

            int LinuxJobManager::startJob(const std::string& command) {
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    setpgid(0, 0); // Put in its own process group

                    // Parse command
                    std::vector<std::string> tokens;
                    std::string token;
                    std::istringstream tokenStream(command);
                    while (std::getline(tokenStream, token, ' ')) {
                        tokens.push_back(token);
                    }
                    std::vector<char*> args;
                    for (auto& s : tokens) args.push_back(&s[0]);
                    args.push_back(nullptr);

                    // Restore default signal handlers
                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    signal(SIGTTIN, SIG_DFL);
                    signal(SIGTTOU, SIG_DFL);
                    signal(SIGCHLD, SIG_DFL);

                    execvp(args[0], args.data());
                    perror("execvp");
                    exit(1);
                } else if (pid < 0) {
                    perror("fork");
                    return -1;
                }

                // Parent process
                setpgid(pid, pid); // Ensure process group is set

                Job job;
                job.jobId = nextJobId++;
                job.command = command;
                job.pid = pid;
                job.running = true;
                job.background = true; // Started as background job by default? Or foreground?
                // The interface startJob implies "start a job".
                // If we want it in background, we don't give it terminal control.
                
                jobs[job.jobId] = job;
                std::cout << "[" << job.jobId << "] " << pid << std::endl;
                return job.jobId;
            }

            bool LinuxJobManager::stopJob(int jobId) {
                if (jobs.find(jobId) == jobs.end()) return false;
                kill(-jobs[jobId].pid, SIGTERM); // Kill process group
                // Wait?
                return true;
            }

            bool LinuxJobManager::bringToForeground(int jobId) {
                if (jobs.find(jobId) == jobs.end()) return false;
                Job& job = jobs[jobId];

                tcsetpgrp(STDIN_FILENO, job.pid); // Give terminal control
                
                if (!job.running) {
                    kill(-job.pid, SIGCONT);
                    job.running = true;
                }

                // Wait for it
                int status;
                waitpid(job.pid, &status, WUNTRACED);

                tcsetpgrp(STDIN_FILENO, getpid()); // Take back terminal control
                tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_tmodes); // Restore modes

                if (WIFSTOPPED(status)) {
                    std::cout << "\n[" << job.jobId << "]+  Stopped                 " << job.command << "\n";
                    job.running = false;
                    return true;
                } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    jobs.erase(jobId);
                }
                return true;
            }

            bool LinuxJobManager::continueInBackground(int jobId) {
                if (jobs.find(jobId) == jobs.end()) return false;
                Job& job = jobs[jobId];

                kill(-job.pid, SIGCONT);
                job.running = true;
                job.background = true;
                return true;
            }

            std::vector<TermiDashJobInfo> LinuxJobManager::listJobs() {
                std::vector<TermiDashJobInfo> list;
                for (auto it = jobs.begin(); it != jobs.end(); ) {
                    int status;
                    pid_t result = waitpid(it->second.pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
                    if (result == 0) {
                        // Running
                    } else if (result > 0) {
                        if (WIFEXITED(status) || WIFSIGNALED(status)) {
                            it = jobs.erase(it);
                            continue;
                        } else if (WIFSTOPPED(status)) {
                            it->second.running = false;
                        } else if (WIFCONTINUED(status)) {
                            it->second.running = true;
                        }
                    } else {
                        // Error or no child
                        it = jobs.erase(it);
                        continue;
                    }

                    TermiDashJobInfo info;
                    info.jobId = it->second.jobId;
                    info.command = it->second.command;
                    info.pid = (unsigned long)it->second.pid;
                    info.status = it->second.running ? "Running" : "Stopped";
                    list.push_back(info);
                    ++it;
                }
                return list;
            }

        }
    }
}
