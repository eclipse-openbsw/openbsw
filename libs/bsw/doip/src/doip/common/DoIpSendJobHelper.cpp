/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpSendJobHelper.h"

#include "doip/common/DoIpHeader.h"

namespace doip
{
::etl::span<uint8_t const> DoIpSendJobHelper::prepareHeaderBuffer(
    ::etl::span<uint8_t> const buffer,
    uint8_t const protocolVersion,
    uint16_t const payloadType,
    uint32_t const payloadLength)
{
    if (buffer.size() < DoIpConstants::DOIP_HEADER_LENGTH)
    {
        return {};
    }

    auto headerBuffer              = buffer.reinterpret_as<DoIpHeader>();
    DoIpHeader& header             = headerBuffer[0];
    header.protocolVersion         = protocolVersion;
    header.invertedProtocolVersion = static_cast<uint8_t>(~protocolVersion);
    header.payloadType             = payloadType;
    header.payloadLength           = payloadLength;

    return buffer.subspan(0U, DoIpConstants::DOIP_HEADER_LENGTH);
}

} // namespace doip
