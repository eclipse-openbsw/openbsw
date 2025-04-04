// Copyright 2024 Accenture.

/**
 * \ingroup transport
 */
#pragma once

#include "transport/TransportMessage.h"

namespace transport
{
/**
 *
 */
template<uint32_t PAYLOAD_SIZE>
class BufferedTransportMessage : public TransportMessage
{
public:
    BufferedTransportMessage();

private:
    uint8_t fBuffer[PAYLOAD_SIZE];
};

template<uint32_t PAYLOAD_SIZE>
inline BufferedTransportMessage<PAYLOAD_SIZE>::BufferedTransportMessage() : TransportMessage()
{
    init(&fBuffer[0], PAYLOAD_SIZE);
}

} // namespace transport
