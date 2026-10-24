/********************************************************************************
 * Copyright (c) 2026 KPIT Group
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

namespace middlewaresample
{

/**
 * What the middleware SOME/IP bridge sample adds to SomeIpSystem's stack, which
 * it shares. All zero when the sample is not built.
 */
struct SomeIpSampleSizes
{
#ifdef PLATFORM_SUPPORT_SOMEIP_BRIDGE_SAMPLE
    /// Its service port, which its multicast reception joins the group on. The
    /// socket and the group membership are each an lwIP UDP PCB (see lwipopts.h).
    static constexpr uint32_t UDP_SOCKETS            = 1U;
    /// The peer node it consumes from.
    static constexpr uint32_t REMOTE_ENDPOINTS       = 1U;
    /// Subscriptions to its two eventgroups.
    static constexpr uint32_t REMOTE_SUBSCRIPTIONS   = 2U;
    /// Its service and its two eventgroups.
    static constexpr uint32_t LOCAL_SERVICES         = 3U;
    /// The find for the peer's instance, and the subscription to its eventgroup.
    static constexpr uint32_t LOCAL_QUERIES          = 2U;
    /// The subscribers its events go to.
    static constexpr uint32_t SUBSCRIPTION_ENDPOINTS = 2U;
    /// The multicast group the peer's events arrive on.
    static constexpr uint32_t MULTICAST_RECEPTIONS   = 1U;
#else
    static constexpr uint32_t UDP_SOCKETS            = 0U;
    static constexpr uint32_t REMOTE_ENDPOINTS       = 0U;
    static constexpr uint32_t REMOTE_SUBSCRIPTIONS   = 0U;
    static constexpr uint32_t LOCAL_SERVICES         = 0U;
    static constexpr uint32_t LOCAL_QUERIES          = 0U;
    static constexpr uint32_t SUBSCRIPTION_ENDPOINTS = 0U;
    static constexpr uint32_t MULTICAST_RECEPTIONS   = 0U;
#endif
};

} // namespace middlewaresample
