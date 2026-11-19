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

#include <etl/array.h>

#include <cstddef>
#include <cstdint>

namespace transport
{
/**
 * Saturating counters of a TransportRouter, per route and for the router as a whole.
 */
class TransportRouterStatistics
{
public:
    static constexpr size_t MAX_ROUTES = 16U;

    enum class RouteCounter : uint8_t
    {
        REQUESTS,       ///< physical requests forwarded to the route
        RESPONSES,      ///< final responses forwarded to a tester
        PENDING,        ///< response pending (NRC 0x78) forwarded
        TIMEOUTS,       ///< no response within P2 / P2*
        NACK_BUSY,      ///< rejected: a request to the route is outstanding
        NACK_TOO_LARGE, ///< rejected: larger than the route's maximum length
        TX_FAILURES,    ///< the route's transport layer did not deliver the request
        DISCARDED,      ///< late or unsolicited responses
        COUNT
    };

    enum class RouterCounter : uint8_t
    {
        LOCAL_REQUESTS,
        FUNCTIONAL_REQUESTS,
        UNKNOWN_TARGET,
        NO_BUFFER,
        COUNT
    };

    static constexpr size_t ROUTE_COUNTERS  = static_cast<size_t>(RouteCounter::COUNT);
    static constexpr size_t ROUTER_COUNTERS = static_cast<size_t>(RouterCounter::COUNT);

    void count(size_t routeIndex, RouteCounter counter);
    void count(RouterCounter counter);
    uint16_t get(size_t routeIndex, RouteCounter counter) const;
    uint16_t get(RouterCounter counter) const;
    void reset();

private:
    static void increment(uint16_t& value);

    ::etl::array<::etl::array<uint16_t, ROUTE_COUNTERS>, MAX_ROUTES> _route{};
    ::etl::array<uint16_t, ROUTER_COUNTERS> _router{};
};

} // namespace transport
