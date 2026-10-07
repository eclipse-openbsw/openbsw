/********************************************************************************
 * Copyright (c) 2026 Jefferson Nascimento
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// AI disclosure: this file was largely generated with an AI assistant and was reviewed and
// tested by the contributor. Assisted-by: Anthropic Claude Opus 5.5

#pragma once

#include <cstddef>

namespace transport
{
/**
 * Outcome of routed requests, e.g. for a lost-communication monitor. Called in the context
 * that observed the outcome; implementations must not block.
 */
class IRouteObserver
{
public:
    IRouteObserver()                                 = default;
    IRouteObserver(IRouteObserver const&)            = delete;
    IRouteObserver& operator=(IRouteObserver const&) = delete;

    /// The node answered a request (final response, not NRC 0x78).
    virtual void routeResponded(size_t routeIndex) = 0;
    /// The node did not answer within P2/P2*, or the request could not be delivered.
    virtual void routeTimedOut(size_t routeIndex)  = 0;

protected:
    ~IRouteObserver() = default;
};

} // namespace transport
