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

#include "transport/routing/TransportRouterStatistics.h"

namespace transport
{
void TransportRouterStatistics::increment(uint16_t& value)
{
    if (value < 0xFFFFU)
    {
        ++value;
    }
}

void TransportRouterStatistics::count(size_t const routeIndex, RouteCounter const counter)
{
    if (routeIndex < MAX_ROUTES)
    {
        increment(_route[routeIndex][static_cast<size_t>(counter)]);
    }
}

void TransportRouterStatistics::count(RouterCounter const counter)
{
    increment(_router[static_cast<size_t>(counter)]);
}

uint16_t TransportRouterStatistics::get(size_t const routeIndex, RouteCounter const counter) const
{
    return (routeIndex < MAX_ROUTES) ? _route[routeIndex][static_cast<size_t>(counter)] : 0U;
}

uint16_t TransportRouterStatistics::get(RouterCounter const counter) const
{
    return _router[static_cast<size_t>(counter)];
}

void TransportRouterStatistics::reset()
{
    for (auto& route : _route)
    {
        route.fill(0U);
    }
    _router.fill(0U);
}

} // namespace transport
