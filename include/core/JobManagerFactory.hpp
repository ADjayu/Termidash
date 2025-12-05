#pragma once

#include "JobManager.hpp"
#include <memory>

std::unique_ptr<IJobManager> createJobManager();