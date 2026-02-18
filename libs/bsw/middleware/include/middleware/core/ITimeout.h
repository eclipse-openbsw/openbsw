// Copyright 2025 BMW AG

#pragma once

namespace middleware
{
namespace core
{
/**
 * \brief Interface for timeout management.
 * \details This interface defines the contract for objects that need to handle timeout
 * updates. Implementations of this interface can register with cluster connections to
 * receive periodic timeout notifications and process expired timeouts accordingly.
 */
struct ITimeout
{
    /**
     * \brief Update all managed timeouts.
     * \details This method is called periodically to check for expired timeouts and trigger
     * appropriate timeout handling. Implementations should check all managed timers and
     * process any that have expired.
     */
    virtual void updateTimeouts() = 0;

    ITimeout& operator=(ITimeout const&) = delete;
    ITimeout& operator=(ITimeout&&)      = delete;
};
} // namespace core
} // namespace middleware
