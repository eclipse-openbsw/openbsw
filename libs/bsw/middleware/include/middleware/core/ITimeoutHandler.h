/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

namespace middleware::core
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
} // namespace middleware::core
