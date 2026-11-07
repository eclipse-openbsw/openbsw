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

#include "middleware/core/Message.h"

#include <cstdint>

namespace middleware::bindings::someip
{

namespace internal
{

/**
 * What a SomeIpConnection needs from each service it carries: which service and
 * instances it is, a way to send that service's messages to the wire, and the
 * start and stop of discovery for its consumed instance.
 */
class IServiceEndpoint
{
public:
    virtual ~IServiceEndpoint() = default;

    virtual uint16_t getServiceId() const          = 0;
    virtual uint16_t getProvidedInstanceId() const = 0;
    virtual uint16_t getConsumedInstanceId() const = 0;

    /** Sends one middleware message of this service to the wire. */
    virtual bool send(::middleware::core::Message const& msg) = 0;

    /** Starts SD FindService and Subscribe for the consumed instance. */
    virtual bool startQuerying() = 0;
    virtual void stopQuerying()  = 0;
};

} // namespace internal

} // namespace middleware::bindings::someip
