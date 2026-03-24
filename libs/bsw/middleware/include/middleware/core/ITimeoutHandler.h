// Copyright 2025 BMW AG

#pragma once

namespace middleware
{
namespace core
{
/**
 * Abstract base class for timeout handling.
 * Implementations can register with cluster connections to receive periodic timeout
 * notifications and process expired timeouts accordingly.
 */
class ITimeoutHandler
{
public:
    /**
     * Update all managed timeouts.
     * This method is called periodically to check for expired timeouts and trigger
     * appropriate timeout handling. Implementations should check all managed timers and
     * process any that have expired.
     */
    virtual void updateTimeouts() = 0;

    ITimeoutHandler& operator=(ITimeoutHandler const&) = delete;
    ITimeoutHandler& operator=(ITimeoutHandler&&)      = delete;

protected:
    ~ITimeoutHandler() = default;
};
} // namespace core
} // namespace middleware
