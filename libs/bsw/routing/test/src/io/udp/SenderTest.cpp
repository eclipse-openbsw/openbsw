/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "io/udp/Sender.h"

#include <io/MemoryQueue.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>

namespace
{
struct SenderTest : ::testing::Test
{
    static constexpr size_t MAX_ELEMENT_SIZE = 1416U;
    static constexpr size_t CAPACITY         = 10 * MAX_ELEMENT_SIZE;
    using Queue                              = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader                             = ::io::MemoryQueueReader<Queue>;
    using Writer                             = ::io::MemoryQueueWriter<Queue>;

    SenderTest() : queue(), reader(queue), writer(queue), socketMock() {}

    void simulatePduReception(uint32_t id, ::etl::span<uint8_t const> data)
    {
        size_t const HEADER = 8U;
        auto memory         = writer.allocate(HEADER + data.size());
        ASSERT_FALSE(memory.empty());
        memory.take<::etl::be_uint32_t>() = id;
        memory.take<::etl::be_uint32_t>() = data.size();
        ASSERT_TRUE(::etl::copy(data, memory));
        writer.commit();
    }

    Queue queue;
    Reader reader = Reader(queue);
    Writer writer = Writer(queue);
    ::udp::AbstractDatagramSocketMock socketMock;
};

/**
 * \desc: Check that sending a UDP packet works.
 */
TEST_F(SenderTest, run)
{
    uint8_t const raw_data[] = {1, 2, 3, 4};
    ::etl::span<uint8_t const> const data(raw_data);
    ::io::udp::Sender sender(reader, socketMock);

    auto element = reader.peek();
    ASSERT_TRUE(element.empty());

    EXPECT_CALL(
        socketMock, send(::testing::Matcher<::etl::span<uint8_t const> const&>(::testing::_)))
        .WillRepeatedly(
            ::testing::Return(::udp::AbstractDatagramSocketMock::ErrorCode::UDP_SOCKET_OK));

    simulatePduReception(1, data);

    element = reader.peek();
    ASSERT_NE(element.size(), 0);

    uint8_t expected[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04};
    ASSERT_THAT(expected, ::testing::ElementsAreArray(element.data(), element.size()));

    sender.run(1);

    element = reader.peek();
    ASSERT_EQ(element.size(), 0);
    EXPECT_EQ(0, sender.socketErrorPdus());
}

/**
 * \desc
 * UDP packet is dropped if there is a socket error
 */
TEST_F(SenderTest, send_udp_packet_with_socket_error)
{
    uint8_t const raw_data[] = {1, 2, 3, 4};
    ::etl::span<uint8_t const> const data(raw_data);
    ::io::udp::Sender sender(reader, socketMock);

    auto element = reader.peek();
    ASSERT_TRUE(element.empty());

    EXPECT_CALL(
        socketMock, send(::testing::Matcher<::etl::span<uint8_t const> const&>(::testing::_)))
        .WillOnce(
            ::testing::Return(::udp::AbstractDatagramSocketMock::ErrorCode::UDP_SOCKET_NOT_OK))
        .WillRepeatedly(
            ::testing::Return(::udp::AbstractDatagramSocketMock::ErrorCode::UDP_SOCKET_OK));

    simulatePduReception(1, data);

    element = reader.peek();
    ASSERT_NE(element.size(), 0);

    uint8_t expected[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04};
    ASSERT_THAT(expected, ::testing::ElementsAreArray(element.data(), element.size()));

    sender.run(1);

    element = reader.peek();
    ASSERT_EQ(element.size(), 0);
    EXPECT_EQ(1, sender.socketErrorPdus());
}

} // anonymous namespace
