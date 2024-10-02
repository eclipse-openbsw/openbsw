// Copyright 2024 Accenture.

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
    uint8_t sourceId, uint8_t targetId, ::estd::slice<uint8_t const> data, uint32_t maxSize)
: TransportMessageWithBuffer((0 == maxSize) ? static_cast<uint32_t>(data.size()) : maxSize)
{
    m.append(data.data(), static_cast<uint16_t>(data.size()));
    m.setSourceId(sourceId);
    m.setTargetId(targetId);
    m.setPayloadLength(static_cast<uint16_t>(data.size()));
}

} // namespace test
} // namespace transport
