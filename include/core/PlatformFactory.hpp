#pragma once
#include <memory>
#include "platform/interfaces/ITerminal.hpp"
#include "platform/interfaces/IProcessManager.hpp"

namespace termidash {

std::unique_ptr<platform::ITerminal> createTerminal();
std::unique_ptr<platform::IProcessManager> createProcessManager();

} // namespace termidash
