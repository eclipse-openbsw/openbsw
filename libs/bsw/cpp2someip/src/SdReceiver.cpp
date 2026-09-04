/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdReceiver.h"

#include "someip/ISdMessageParser.h"
#include "someip/NetworkChannel.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/Statistics.h"
#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

SdReceiver::SdReceiver(ISdMessageParser& parser) : _parser(parser) {}

void SdReceiver::received(NetworkChannel& channel, uint32_t const length)
{
    Statistics::incCounter(Statistics::Counter::SD_FRAME_RX);

    auto const input = channel.getInputBuffer();

    uint32_t const bigLength = length;
    uint32_t offset          = 0;
    while (offset < bigLength)
    {
        uint32_t const bytesLeft = bigLength - offset;
        if (bytesLeft < SomeIpMessage::OFFSET_PAYLOAD)
        {
            WARN_LOG(
                SOMEIP, "SdReceiver::received(): %d bytes left, but no valid header", bytesLeft);
            return;
        }

        Statistics::incCounter(Statistics::Counter::SD_MESSAGE_RX);

        SomeIpMessage const message(input.subspan(offset, bytesLeft));

        uint32_t const payloadLength = message.getPayloadLength();
        if (bytesLeft < (payloadLength + SomeIpMessage::OFFSET_PAYLOAD))
        {
            WARN_LOG(
                SOMEIP,
                "SdReceiver::received(): %d bytes left, but expected is %d",
                bytesLeft,
                payloadLength + SomeIpMessage::OFFSET_PAYLOAD);
            Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
            return;
        }

        if (payloadLength > MAX_SD_PAYLOAD_LENGTH)
        {
            WARN_LOG(SOMEIP, "SdReceiver::received(): payload too long (%d bytes)", payloadLength);
            Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
            return;
        }

        handleMessage(channel, message);

        offset += (payloadLength + SomeIpMessage::OFFSET_PAYLOAD);
    }
}

// private
void SdReceiver::handleMessage(NetworkChannel& channel, SomeIpMessage const& message)
{
    if (message.getMessageId() == SD_MESSAGE_ID)
    {
        _parser.handleMessage(message, channel.getRemoteEndpoint(), channel.isMulticast());
    }
    else
    {
        WARN_LOG(
            SOMEIP, "SdReceiver::handleMessage() dropping message 0x%x", message.getMessageId());
        Statistics::incCounter(Statistics::Counter::SD_MALFORMED_MESSAGE_RX);
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
