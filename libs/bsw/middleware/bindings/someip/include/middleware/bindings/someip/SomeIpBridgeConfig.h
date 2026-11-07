/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "ip/IPAddress.h"
#include "someip/SdConfig.h"
#include "someip/SomeIpConstants.h"

#include <cstdint>

namespace middleware::bindings::someip
{

/**
 * SOME/IP network parameters the middleware deployment has no vocabulary for.
 * Supplied by whoever wires the endpoint into an application, from the service's
 * SOME/IP deployment.
 */
struct SomeIpBridgeConfig
{
    ::someip::major_version::type majorVersion{1U};
    ::someip::minor_version::type minorVersion{1U};
    ::someip::instance_id::type instanceId{1U};

    /**
     * Port this node offers the service on, and receives the consumed instance's
     * events on.
     */
    uint16_t providerPort{0U};

    /**
     * Peer instance a proxy of this service consumes, and the eventgroup it
     * subscribes to. Unused if no proxy subscribes.
     */
    ::someip::instance_id::type consumedInstanceId{0U};
    ::someip::eventgroup_id::type consumedEventGroupId{0U};

    uint8_t proto{::someip::proto::SD_L4_PROTO_UDP};
    ::someip::ttl::type ttl{::someip::ttl::type{}};

    /** SD announcement timing. Left default, this is the stack's own ramp. */
    ::someip::SdOfferConfig sdConfig{};

    /** SD FindService timing for the consumed instance. */
    ::someip::SdConfig findConfig{};

    /**
     * How long a request to the consumed instance waits for its response
     * before the proxy is answered with an error.
     */
    uint32_t requestTimeoutMs{1000U};

    /**
     * SOME/IP client id of the first request channel to the consumed instance.
     * The bridge uses MaxPending consecutive ids, one per channel, which is how
     * the stack tells their responses apart.
     */
    uint16_t clientId{1U};

    /**
     * Group the multicast-flagged events publish to. The same endpoint must be
     * registered on the provided eventgroup, since that is what puts the multicast
     * option in the Ack and so tells a subscriber where to listen. Unset leaves
     * every event unicast.
     */
    ::ip::IPAddress eventMulticastAddress{};
    uint16_t eventMulticastPort{0U};
};

} // namespace middleware::bindings::someip
