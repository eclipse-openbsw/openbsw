/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/bindings/someip/PayloadTransfer.h"

#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/bindings/someip/PayloadSlot.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/types.h"

namespace middleware::bindings::someip::internal
{

using ::middleware::core::Message;
using ::middleware::core::MessagePayloadBuilder;

void serializeInPlace(Message const& msg, IPayloadSlot& slot)
{
    auto const raw = MessagePayloadBuilder::readRawPayload(msg);
    // Slot and message payload have the same type by construction.
    slot.serializeFrom(raw.data());
}

bool loadPayloadFromSlot(Message& msg, IPayloadSlot const& slot)
{
    return slot.allocateInto(msg) == ::middleware::core::HRESULT::Ok;
}

void dispatchAndRelease(ISomeIpConnection const& connection, Message& msg, bool const hasPayload)
{
    static_cast<void>(connection.dispatchMessage(msg));
    if (hasPayload)
    {
        MessagePayloadBuilder::deallocate(msg);
    }
}

void allocateAndDispatch(
    ISomeIpConnection const& connection, Message& msg, IPayloadSlot const& slot)
{
    if (!loadPayloadFromSlot(msg, slot))
    {
        return; // allocator exhausted; drop rather than dispatch a blank
    }
    dispatchAndRelease(connection, msg, /*hasPayload=*/true);
}

} // namespace middleware::bindings::someip::internal
