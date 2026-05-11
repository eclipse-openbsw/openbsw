/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "io/udp/Receiver.h"

#include <io/MemoryQueue.h>
#include <ip/IPAddress.h>
#include <routing/ErrorHandler.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>

using namespace ::testing;

namespace
{
struct ReceiverTest : Test
{
    static constexpr size_t NUMBER_OF_REMOTE_IP_ADDRESSES = 3;
    static constexpr size_t MAX_ELEMENT_SIZE              = 1416U;
    static constexpr size_t CAPACITY                      = 10 * MAX_ELEMENT_SIZE;
    using Queue  = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Queue>;
    using Writer = ::io::MemoryQueueWriter<Queue>;

    ReceiverTest()
    : socketMock()
    , queue()
    , reader(queue)
    , writer(queue)
    , errorHandler(
          ::routing::ErrorHandler::Function::create<ReceiverTest, &ReceiverTest::handleError>(
              *this),
          0U)
    , remoteIpAddresses(
          {::ip::make_ip4(0, 0, 0, 0), ::ip::make_ip4(1, 1, 1, 1), ::ip::make_ip4(2, 2, 2, 2)})
    , receiver(
          socketMock,
          writer,
          errorHandler,
          ::etl::make_span(remoteIpAddresses).reinterpret_as<uint8_t>())
    , payload()
    , bytesRead(0)
    , invalidRemoteIpAddressCounter(0)
    , memAllocationFailureCounter(0)
    {
        ON_CALL(socketMock, read(_, _))
            .WillByDefault(
                [this](uint8_t* buffer, size_t n) -> size_t
                {
                    (void)read(buffer, n);
                    return bytesRead;
                });
    }

    void simulateUdpPacketReception(
        ::etl::span<uint8_t const> const udpPayload,
        ::ip::IPAddress const sourceAddress = ::ip::make_ip4(0, 0, 0, 0))
    {
        ASSERT_LE(udpPayload.size(), 0xFFFF);
        payload                 = udpPayload;
        auto destinationAddress = ::ip::make_ip4(127, 0, 0, 1);
        uint16_t port           = 2222U;
        socketMock.getDataListener()->dataReceived(
            socketMock, sourceAddress, port, destinationAddress, udpPayload.size());
    }

    void read(uint8_t* buffer, size_t n)
    {
        if (buffer == nullptr || n == 0)
        {
            bytesRead = 0;
            return;
        }

        auto toBeRead = payload;
        if (n != toBeRead.size())
        {
            toBeRead = toBeRead.first(n);
        }
        ASSERT_TRUE(::etl::copy(toBeRead, ::etl::span<uint8_t>(buffer, n)));
        payload   = payload.subspan(toBeRead.size());
        bytesRead = toBeRead.size();
    }

    void handleError(::routing::ErrorHandler::StatusCode const statusCode, uint8_t, uint32_t)
    {
        switch (statusCode)
        {
            case ::routing::ErrorHandler::StatusCode::INVALID_REMOTE_IP_ADDRESS:
                ++invalidRemoteIpAddressCounter;
                break;
            case ::routing::ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE:
                ++memAllocationFailureCounter;
            default: break;
        }
    }

    NiceMock<::udp::AbstractDatagramSocketMock> socketMock;

    Queue queue;
    Reader reader;
    Writer writer;

    ::routing::ErrorHandler errorHandler;
    ::etl::array<::ip::IPAddress, NUMBER_OF_REMOTE_IP_ADDRESSES> remoteIpAddresses;
    ::io::udp::Receiver receiver;

    ::etl::span<uint8_t const> payload;
    size_t bytesRead;
    uint32_t invalidRemoteIpAddressCounter;
    uint32_t memAllocationFailureCounter;
};

/**
 * \desc: Check that reception of a UDP packet with a single PDU works.
 */
TEST_F(ReceiverTest, receive_udp_packet_with_single_pdu)
{
    uint8_t const udpPayload[] = {
        0x00,
        0x00,
        0x00,
        0x02, // pduTransport id
        0x00,
        0x00,
        0x00,
        0x02, // size
        0xff,
        0xaa // data
    };

    auto element = reader.peek();
    ASSERT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload);

    element = reader.peek();
    EXPECT_EQ(10, element.size());

    reader.release();
    element = reader.peek();
    EXPECT_EQ(0, element.size());
    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(0, receiver.failedMemAllocPdus());
}

/**
 * \desc: Check that reception of a UDP packet with multiple PDU works.
 */
TEST_F(ReceiverTest, receive_udp_packet_with_multiple_pdus)
{
    uint8_t const udpPayload[] = {
        0x00, 0x00, 0x00, 0x02, // pduTransport id
        0x00, 0x00, 0x00, 0x02, // size
        0xff, 0xaa,             // data
        0x00, 0x00, 0x00, 0x02, // pduTransport id
        0x00, 0x00, 0x00, 0x02, // size
        0xff, 0xaa              // data
    };

    auto element = reader.peek();
    ASSERT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload);

    element = reader.peek();
    EXPECT_EQ(20, element.size());

    reader.release();
    element = reader.peek();
    EXPECT_EQ(0, element.size());
    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(0, receiver.failedMemAllocPdus());
}

/**
 * \desc: Check that reception of UDP packets from different sources works.
 */
TEST_F(ReceiverTest, receive_udp_packet_from_different_sources)
{
    uint8_t const udpPayload[] = {
        0x00,
        0x00,
        0x00,
        0x02, // pduTransport id
        0x00,
        0x00,
        0x00,
        0x02, // size
        0xff,
        0xaa // data
    };

    auto element = reader.peek();
    ASSERT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload, ::ip::make_ip4(1, 1, 1, 1));

    element = reader.peek();
    EXPECT_EQ(10, element.size());

    reader.release();
    element = reader.peek();
    EXPECT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload, ::ip::make_ip4(2, 2, 2, 2));

    element = reader.peek();
    EXPECT_EQ(10, element.size());

    reader.release();
    element = reader.peek();
    EXPECT_EQ(0, element.size());
    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(0, receiver.failedMemAllocPdus());
}

/**
 * \desc: Check that reception of a UDP packet with no remote IP address works.
 */
TEST_F(ReceiverTest, receive_udp_packet_no_remote_ip_addresses)
{
    new (&receiver)::io::udp::Receiver(socketMock, writer, errorHandler);

    uint8_t const udpPayload[] = {
        0x00,
        0x00,
        0x00,
        0x02, // pduTransport id
        0x00,
        0x00,
        0x00,
        0x02, // size
        0xff,
        0xaa // data
    };

    auto element = reader.peek();
    ASSERT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload, ::ip::make_ip4(3, 3, 3, 3));

    element = reader.peek();
    EXPECT_EQ(10, element.size());

    reader.release();
    element = reader.peek();
    EXPECT_EQ(0, element.size());
    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(0, receiver.failedMemAllocPdus());
}

/**
 * \desc: Check that reception of a UDP packet from an invalid source fails.
 */
TEST_F(ReceiverTest, receive_udp_packet_from_invalid_source)
{
    uint8_t const udpPayload[] = {
        0x00,
        0x00,
        0x00,
        0x02, // pduTransport id
        0x00,
        0x00,
        0x00,
        0x02, // size
        0xff,
        0xaa // data
    };

    auto element = reader.peek();
    ASSERT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload, ::ip::make_ip4(3, 3, 3, 3));

    element = reader.peek();
    EXPECT_EQ(0, element.size());
    EXPECT_EQ(1, invalidRemoteIpAddressCounter);
    EXPECT_EQ(1, receiver.invalidIpAddressPdus());
    EXPECT_EQ(0, receiver.failedMemAllocPdus());
}

/**
 * \desc: Check that reception of a UDP packet fails if memory allocation does not succeed.
 */
TEST_F(ReceiverTest, receive_udp_packet_with_memory_allocation_failure)
{
    constexpr size_t NEW_MAX_ELEMENT_SIZE = 2U;
    constexpr size_t NEW_CAPACITY         = 10 * NEW_MAX_ELEMENT_SIZE;
    using NewQueue                        = ::io::MemoryQueue<NEW_CAPACITY, NEW_MAX_ELEMENT_SIZE>;
    using NewWriter                       = ::io::MemoryQueueWriter<NewQueue>;
    NewQueue newQueue;
    NewWriter newWriter(newQueue);
    new (&receiver)::io::udp::Receiver(socketMock, newWriter, errorHandler);

    uint8_t const udpPayload[] = {
        0x00,
        0x00,
        0x00,
        0x02, // pduTransport id
        0x00,
        0x00,
        0x00,
        0x04, // size
        0xff,
        0xaa,
        0xff,
        0xaa // data
    };

    auto element = reader.peek();
    ASSERT_EQ(0, element.size());

    simulateUdpPacketReception(udpPayload);

    element = reader.peek();
    EXPECT_EQ(0, element.size());
    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(1, receiver.failedMemAllocPdus());
    EXPECT_EQ(1, memAllocationFailureCounter);
}

/**
 * \desc
 * If the queue is full twice in a row, the error handling is only called once.
 */
TEST_F(ReceiverTest, multiple_drops_one_error_handling)
{
    uint8_t const udpPayload[] = {
        0x00,
        0x00,
        0x00,
        0x02, // pduTransport id
        0x00,
        0x00,
        0x00,
        0x04, // size
        0xff,
        0xaa,
        0xff,
        0xaa // data
    };

    constexpr size_t NEW_MAX_ELEMENT_SIZE = sizeof(udpPayload);
    constexpr size_t NEW_CAPACITY         = sizeof(Queue::size_type) + NEW_MAX_ELEMENT_SIZE;
    using NewQueue                        = ::io::MemoryQueue<NEW_CAPACITY, NEW_MAX_ELEMENT_SIZE>;
    using NewWriter                       = ::io::MemoryQueueWriter<NewQueue>;
    using NewReader                       = ::io::MemoryQueueReader<NewQueue>;
    NewQueue newQueue;
    NewWriter newWriter(newQueue);
    NewReader newReader(newQueue);
    new (&receiver)::io::udp::Receiver(socketMock, newWriter, errorHandler);

    simulateUdpPacketReception(udpPayload); // works
    simulateUdpPacketReception(udpPayload); // fails

    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(1, receiver.failedMemAllocPdus());
    EXPECT_EQ(1, memAllocationFailureCounter);

    simulateUdpPacketReception(udpPayload); // fails

    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(2, receiver.failedMemAllocPdus());
    EXPECT_EQ(1, memAllocationFailureCounter); // not updated

    newReader.release();

    simulateUdpPacketReception(udpPayload); // works
    simulateUdpPacketReception(udpPayload); // fails

    EXPECT_EQ(0, receiver.invalidIpAddressPdus());
    EXPECT_EQ(3, receiver.failedMemAllocPdus());
    EXPECT_EQ(2, memAllocationFailureCounter); // updated
}
} // anonymous namespace
