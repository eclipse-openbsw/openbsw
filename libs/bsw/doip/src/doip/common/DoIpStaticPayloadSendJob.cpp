/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpStaticPayloadSendJob.h"

namespace doip
{
using ::etl::span;

DoIpStaticPayloadSendJob::DoIpStaticPayloadSendJob(
    uint8_t const protocolVersion,
    uint16_t const payloadType,
    span<uint8_t const> const payload,
    ReleaseCallbackType const releaseCallback)
: DoIpSimplePayloadSendJob(
    protocolVersion, payloadType, static_cast<uint16_t>(payload.size()), releaseCallback)
, _payload(payload)
{}

span<uint8_t const>
DoIpStaticPayloadSendJob::getPayloadBuffer(span<uint8_t> const /* staticBuffer */) const
{
    return _payload;
}

} // namespace doip
