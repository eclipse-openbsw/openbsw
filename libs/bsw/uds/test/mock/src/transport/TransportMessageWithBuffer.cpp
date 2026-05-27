/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "transport/TransportMessageWithBuffer.h"

namespace transport
{
namespace test
{
TransportMessageWithBuffer::TransportMessageWithBuffer(uint32_t size) : buffer(size)
{
    m.init(buffer.data(), size);
}

TransportMessageWithBuffer::TransportMessageWithBuffer(
    uint8_t sourceAddress, uint8_t targetId, ::etl::span<uint8_t const> data, uint32_t maxSize)
: TransportMessageWithBuffer((0 == maxSize) ? static_cast<uint32_t>(data.size()) : maxSize)
{
    m.append(data.data(), static_cast<uint16_t>(data.size()));
    m.setSourceAddress(sourceAddress);
    m.setTargetAddress(targetId);
    m.setPayloadLength(static_cast<uint16_t>(data.size()));
}

} // namespace test
} // namespace transport
