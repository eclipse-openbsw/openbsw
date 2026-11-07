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

#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/core/Message.h"

namespace middleware::bindings::someip
{

namespace internal
{

/**
 * Points \p slot at the Message's own payload rather than copying it in. The
 * stack serializes before sendEvent() or the response closure returns, and
 * write() frees the payload only after that; the caller clears it then.
 */
void serializeInPlace(::middleware::core::Message const& msg, IPayloadSlot& slot);

/** Attaches the value decoded into \p slot to \p msg as its payload. */
bool loadPayloadFromSlot(::middleware::core::Message& msg, IPayloadSlot const& slot);

/**
 * Hands \p msg from the wire to the middleware through \p connection. Dispatch is
 * synchronous, so the payload is consumed by the time this returns, and nothing
 * else frees it: these messages are injected straight into dispatchMessage() from
 * the wire side.
 */
void dispatchAndRelease(
    ISomeIpConnection const& connection, ::middleware::core::Message& msg, bool hasPayload);

/** Attaches the value decoded into \p slot to \p msg and hands it to the middleware. */
void allocateAndDispatch(
    ISomeIpConnection const& connection,
    ::middleware::core::Message& msg,
    IPayloadSlot const& slot);

} // namespace internal

} // namespace middleware::bindings::someip
