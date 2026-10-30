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

#include "middleware/bindings/someip/PayloadSlot.h"
#include "someip/ServiceInfo.h"

#include <etl/span.h>

#include <cstdint>

namespace middleware::bindings::someip
{

/**
 * One row of the bridge's dispatch table.
 * Holds everything needed to move one service member between a middleware member
 * id and the stack. Built once per member at setup, then looked up by member id.
 */
struct MemberBinding
{
    enum class Kind : uint8_t
    {
        Event,           // notification, no request or response
        FireAndForget,   // method, no response
        RequestResponse, // method, request and response
        RequestOnly,     // method, request but no response value
        ResponseOnly     // method, no request but a response value
    };

    // Widest members first, so the table carries no padding between them.

    /** Null where that direction carries no payload. */
    internal::IPayloadSlot* requestSlot{nullptr};
    internal::IPayloadSlot* responseSlot{nullptr};

    /** Eventgroups this member belongs to. An empty span sends unicast only. */
    ::etl::span<uint16_t const> eventGroupIds{};

    /**
     * What the stack is told when this member arrives as a method call. Fixed
     * at bind time, so answering is a lookup, and each member's answer has its
     * own storage rather than one object overwritten per call.
     */
    ::someip::MethodDetail methodDetail{};

    uint16_t memberId{0U};

    /** The provided instance, or the consumed one for a bindConsumedEvent(). */
    uint16_t instanceId{0U};

    Kind kind{Kind::Event};

    /** Publish to the config's multicast group rather than to each subscriber. */
    bool isMulticast{false};
};

} // namespace middleware::bindings::someip
