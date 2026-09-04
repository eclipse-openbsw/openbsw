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

#include "someip/NetworkChannel.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SdMessageParserMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/Statistics.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::someip;

struct SdReceiverTest : Test
{
    SdReceiverTest()
    : _receiver(_parser)
    , _channel(_resource, ::ip::IPEndpoint(::ip::make_ip4(192U, 0U, 2U, 1U), 10U))
    {
        _resource.incRefCounter();
    }

    StrictMock<SdMessageParserMock> _parser;
    SdReceiver _receiver;

    StrictMock<NetworkResourceMock> _resource;
    NetworkChannel _channel;
};

/**
 * Make sure an input length smaller than the valid header size is detected by received(). The
 * message shall be discared without further actions.
 */
TEST_F(SdReceiverTest, received_input_length_smaller_than_header_size)
{
    _receiver.received(_channel, 0U);
    _receiver.received(_channel, 5U);
    _receiver.received(_channel, 11U);
}

/**
 * Make sure an input length smaller than the payload size + valid header size is detected by
 * received(). The message shall be discared without further actions.
 */
TEST_F(SdReceiverTest, received_input_length_smaller_than_header_size_plus_payload_size)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x0F);

    _receiver.received(_channel, 17U);
}

/**
 * Make sure received() detects an input payload exceeding the max payload. The message shall be
 * discared without further actions.
 */
TEST_F(SdReceiverTest, received_payload_exceeding_max_payload)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x58C);

    _receiver.received(_channel, 0x5DBU);
}

/**
 * Make sure received() detects an incoming message that has the wrong message id. The message shall
 * be discared without further actions.
 */
TEST_F(SdReceiverTest, received_invalid_message_id)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x30U);
    message.setMessageId(0U);

    _receiver.received(_channel, 0x38U);
}

/**
 * Make sure received() invokes handleMessage() if message is correctly formed.
 */
TEST_F(SdReceiverTest, received_valid_message)
{
    SomeIpMessage message(_resource.getInputBuffer());
    message.setLength(0x30U);
    message.setMessageId(SD_MESSAGE_ID);

    EXPECT_CALL(_parser, handleMessage(_, _, _)).Times(1);
    EXPECT_CALL(_resource, isOpen()).WillOnce(Return(true));

    _receiver.received(_channel, 0x38U);
}

} // anonymous namespace
