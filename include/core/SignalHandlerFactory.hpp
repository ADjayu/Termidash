#pragma once

#include "core/ISignalHandler.hpp"
#include <memory>

namespace termidash {

std::unique_ptr<ISignalHandler> createSignalHandler();

} // namespace termidash
