/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    uint8_t _buffer[PAYLOAD_SIZE];
};

template<uint32_t PAYLOAD_SIZE>
inline BufferedTransportMessage<PAYLOAD_SIZE>::BufferedTransportMessage() : TransportMessage()
{
    init(&_buffer[0], PAYLOAD_SIZE);
}

} // namespace transport
