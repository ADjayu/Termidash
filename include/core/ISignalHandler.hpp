#pragma once

namespace termidash {

class ISignalHandler {
public:
    virtual ~ISignalHandler() = default;

    // Sets up the signal handlers for the platform
    virtual void setupHandlers() = 0;

    // Resets signal handlers to default behavior
    virtual void resetHandlers() = 0;
};

} // namespace termidash
