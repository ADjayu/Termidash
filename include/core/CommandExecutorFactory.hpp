#pragma once

#include "CommandExecutor.hpp"
#include <memory>

std::unique_ptr<ICommandExecutor> createCommandExecutor();
