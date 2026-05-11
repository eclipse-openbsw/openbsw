/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportIntegration.h"

#include "blob/configuration.h"
#include "routing/ErrorHandler.h"
#include "routing/PduTransportConfig.h"
#include "routing/constants.h"
#include "routing/util.h"

#include <bsp/timer/SystemTimerMock.h>
#include <etl/vector.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>

using namespace ::testing;

namespace
{
class PduTransportIntegrationTest : public ::testing::Test
{
public:
    static constexpr size_t MAX_ELEMENT_SIZE = 1416U;
    static constexpr uint8_t FIRST_PDU_TRANSPORT_CHANNEL_ID
        = ::routing::NUM_CAN_CHANNELS + ::routing::NUM_FLEXRAY_CHANNELS;

    using PduTransportIntegration = ::routing::
        PduTransportIntegration<::routing::NUM_PDU_TRANSPORT_CHANNELS, MAX_ELEMENT_SIZE>;

    PduTransportIntegrationTest() : _pduTransportIntegration(), _timestamp(0)
    {
        ON_CALL(_systemTimerMock, getSystemTimeUs32Bit())
            .WillByDefault([&t = _timestamp] { return t; });

        for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
        {
            auto& inputSocket = _lwipInputSockets.emplace_back();
            ON_CALL(inputSocket, bind(_, _))
                .WillByDefault(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
            ON_CALL(inputSocket, join(_))
                .WillByDefault(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
            _inputSockets[i] = &inputSocket;

            auto& outputSocket = _lwipOutputSockets.emplace_back();
            ON_CALL(outputSocket, bind(_, _))
                .WillByDefault(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
            ON_CALL(outputSocket, connect(_, _, _))
                .WillByDefault(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
            ON_CALL(outputSocket, send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .WillByDefault(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
            _outputSockets[i] = &outputSocket;
        }

        for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
        {
            _pduTransportChannelIds[i] = FIRST_PDU_TRANSPORT_CHANNEL_ID + i;
            auto const channelConfig   = ::routing::config(
                ::blob::CONFIGURATION_BLOB,
                ::blob::Config::Type::CHANNEL,
                _pduTransportChannelIds[i]);
            ::routing::PduTransportConfig pduTransportConfig;
            EXPECT_TRUE(::routing::load(channelConfig.data, pduTransportConfig));

            _modes[i]                = pduTransportConfig.mode;
            _transmissionTimeouts[i] = pduTransportConfig.transmissionTimeout;
        }

        _pduTransportIntegration.init(
            ::blob::CONFIGURATION_BLOB, _pduTransportChannelIds, _inputSockets, _outputSockets);
    }

protected:
    using Socket      = NiceMock<::udp::AbstractDatagramSocketMock>;
    using SystemTimer = NiceMock<SystemTimerMock>;

    void commitToAll(size_t const n)
    {
        for (auto& writer : _pduTransportIntegration.bufferedOutputWriters())
        {
            (void)writer.allocate(n);
            writer.commit();
        }
    }

    void flushAll()
    {
        for (auto& writer : _pduTransportIntegration.bufferedOutputWriters())
        {
            writer.flush();
        }
    }

    void writeToAll(size_t const n)
    {
        commitToAll(n);
        flushAll();
    }

    bool isRx(size_t const i)
    {
        return (_modes[i] == ::routing::PduTransportConfig::Mode::RX_TX)
               || (_modes[i] == ::routing::PduTransportConfig::Mode::RX);
    }

    bool isTx(size_t const i)
    {
        return (_modes[i] == ::routing::PduTransportConfig::Mode::RX_TX)
               || (_modes[i] == ::routing::PduTransportConfig::Mode::TX);
    }

    ::etl::vector<Socket, ::routing::NUM_PDU_TRANSPORT_CHANNELS> _lwipInputSockets;
    ::etl::vector<Socket, ::routing::NUM_PDU_TRANSPORT_CHANNELS> _lwipOutputSockets;

    ::etl::array<::udp::AbstractDatagramSocket*, ::routing::NUM_PDU_TRANSPORT_CHANNELS>
        _inputSockets;
    ::etl::array<::udp::AbstractDatagramSocket*, ::routing::NUM_PDU_TRANSPORT_CHANNELS>
        _outputSockets;

    ::etl::array<uint8_t, ::routing::NUM_PDU_TRANSPORT_CHANNELS> _pduTransportChannelIds;
    ::etl::array<::routing::PduTransportConfig::Mode, ::routing::NUM_PDU_TRANSPORT_CHANNELS> _modes;
    ::etl::array<uint16_t, ::routing::NUM_PDU_TRANSPORT_CHANNELS> _transmissionTimeouts;

    ::routing::PduTransportIntegration<::routing::NUM_PDU_TRANSPORT_CHANNELS, MAX_ELEMENT_SIZE>
        _pduTransportIntegration;

    ::ip::IPAddress const _ipAddress = ::ip::make_ip4(0, 0, 0, 0);
    uint16_t const _vlanId           = 0U;

    SystemTimer _systemTimerMock;
    uint32_t _timestamp;
};

/**
 * \desc: Queues, readers and writers are created without initialization.
 */
TEST_F(PduTransportIntegrationTest, members_created_without_init)
{
    new (&_pduTransportIntegration) PduTransportIntegration();

    EXPECT_EQ(::routing::NUM_PDU_TRANSPORT_CHANNELS, _pduTransportIntegration.inputQueues().size());
    EXPECT_EQ(
        ::routing::NUM_PDU_TRANSPORT_CHANNELS, _pduTransportIntegration.outputQueues().size());
    EXPECT_EQ(
        ::routing::NUM_PDU_TRANSPORT_CHANNELS, _pduTransportIntegration.inputReaders().size());
    EXPECT_EQ(
        ::routing::NUM_PDU_TRANSPORT_CHANNELS, _pduTransportIntegration.inputWriters().size());
    EXPECT_EQ(
        ::routing::NUM_PDU_TRANSPORT_CHANNELS, _pduTransportIntegration.outputWriters().size());
    EXPECT_EQ(0, _pduTransportIntegration.bufferedOutputWriters().size());
    EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
    EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());
}

/**
 * \desc: Reinitialization has no effect.
 */
TEST_F(PduTransportIntegrationTest, reinit)
{
    _pduTransportIntegration.init({}, {}, {}, {});

    EXPECT_EQ(
        ::routing::NUM_PDU_TRANSPORT_CHANNELS,
        _pduTransportIntegration.bufferedOutputWriters().size());
}

/**
 * \desc: Activating and disabling channels change input and output sockets.
 */
TEST_F(PduTransportIntegrationTest, activate_and_disable)
{
    constexpr size_t N = 3, RX = 5, TX = 6;

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto& inputSocket = _lwipInputSockets[i];
        if (isRx(i))
        {
            EXPECT_CALL(inputSocket, bind(_, _)).Times(N);
            EXPECT_CALL(inputSocket, join(_)).Times(N);
        }
        EXPECT_CALL(inputSocket, close()).Times(N);

        auto& outputSocket = _lwipOutputSockets[i];
        if (isTx(i))
        {
            EXPECT_CALL(outputSocket, bind(_, _)).Times(N);
            EXPECT_CALL(outputSocket, connect(_, _, _)).Times(N);
        }
        EXPECT_CALL(outputSocket, close()).Times(N);
        EXPECT_CALL(outputSocket, disconnect()).Times(N);
    }

    for (size_t i = 0; i < N; ++i)
    {
        _pduTransportIntegration.activate(_ipAddress, _vlanId, {});
        EXPECT_EQ(RX, _pduTransportIntegration.udpReceivers().size());
        EXPECT_EQ(TX, _pduTransportIntegration.udpSenders().size());

        _pduTransportIntegration.disable();
        EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
        EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());
    }
}

/**
 * \desc: Activating and disabling channels change input and output sockets with a smaller number of
 * channels.
 */
TEST_F(PduTransportIntegrationTest, activate_and_disable_smaller_number_of_pdu_transport_channels)
{
    constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = ::routing::NUM_PDU_TRANSPORT_CHANNELS - 1;
    constexpr size_t N = 3, RX = 5, TX = 5;

    new (&_pduTransportIntegration) PduTransportIntegration();

    _pduTransportIntegration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportChannelIds).first(NUM_PDU_TRANSPORT_CHANNELS),
        ::etl::make_span(_inputSockets).first(NUM_PDU_TRANSPORT_CHANNELS),
        ::etl::make_span(_outputSockets).first(NUM_PDU_TRANSPORT_CHANNELS));

    for (size_t i = 0; i < NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto& inputSocket = _lwipInputSockets[i];
        if (isRx(i))
        {
            EXPECT_CALL(inputSocket, bind(_, _)).Times(N);
            EXPECT_CALL(inputSocket, join(_)).Times(N);
        }
        EXPECT_CALL(inputSocket, close()).Times(N);

        auto& outputSocket = _lwipOutputSockets[i];
        if (isTx(i))
        {
            EXPECT_CALL(outputSocket, bind(_, _)).Times(N);
            EXPECT_CALL(outputSocket, connect(_, _, _)).Times(N);
        }
        EXPECT_CALL(outputSocket, close()).Times(N);
        EXPECT_CALL(outputSocket, disconnect()).Times(N);
    }

    for (size_t i = 0; i < N; ++i)
    {
        _pduTransportIntegration.activate(_ipAddress, _vlanId, {});
        EXPECT_EQ(RX, _pduTransportIntegration.udpReceivers().size());
        EXPECT_EQ(TX, _pduTransportIntegration.udpSenders().size());

        _pduTransportIntegration.disable();
        EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
        EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());
    }
}

/**
 * \desc: Activating and disabling channels change input and output sockets with a bigger number of
 * channels.
 */
TEST_F(PduTransportIntegrationTest, activate_and_disable_bigger_number_of_pdu_transport_channels)
{
    constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = ::routing::NUM_PDU_TRANSPORT_CHANNELS + 1;
    constexpr size_t N = 3, RX = 5, TX = 6;

    ::etl::array<uint8_t, NUM_PDU_TRANSPORT_CHANNELS> pduTransportChannelIds;
    ::etl::array<::udp::AbstractDatagramSocket*, NUM_PDU_TRANSPORT_CHANNELS> inputSockets;
    ::etl::array<::udp::AbstractDatagramSocket*, NUM_PDU_TRANSPORT_CHANNELS> outputSockets;
    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        pduTransportChannelIds[i] = _pduTransportChannelIds[i];
        inputSockets[i]           = _inputSockets[i];
        outputSockets[i]          = _outputSockets[i];
    }
    pduTransportChannelIds.back() = _pduTransportChannelIds.back() + 1;
    inputSockets.back()           = nullptr;
    outputSockets.back()          = nullptr;

    new (&_pduTransportIntegration) PduTransportIntegration();

    _pduTransportIntegration.init(
        ::blob::CONFIGURATION_BLOB, pduTransportChannelIds, inputSockets, outputSockets);

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto& inputSocket = _lwipInputSockets[i];
        if (isRx(i))
        {
            EXPECT_CALL(inputSocket, bind(_, _)).Times(N);
            EXPECT_CALL(inputSocket, join(_)).Times(N);
        }
        EXPECT_CALL(inputSocket, close()).Times(N);

        auto& outputSocket = _lwipOutputSockets[i];
        if (isTx(i))
        {
            EXPECT_CALL(outputSocket, bind(_, _)).Times(N);
            EXPECT_CALL(outputSocket, connect(_, _, _)).Times(N);
        }
        EXPECT_CALL(outputSocket, close()).Times(N);
        EXPECT_CALL(outputSocket, disconnect()).Times(N);
    }

    for (size_t i = 0; i < N; ++i)
    {
        _pduTransportIntegration.activate(_ipAddress, _vlanId, {});
        EXPECT_EQ(RX, _pduTransportIntegration.udpReceivers().size());
        EXPECT_EQ(TX, _pduTransportIntegration.udpSenders().size());

        _pduTransportIntegration.disable();
        EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
        EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());
    }
}

/**
 * \desc: No channels are activated if the VLAN ID is invalid.
 */
TEST_F(PduTransportIntegrationTest, activate_invalid_vlan_id)
{
    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto& inputSocket = _lwipInputSockets[i];
        EXPECT_CALL(inputSocket, bind(_, _)).Times(0);
        EXPECT_CALL(inputSocket, join(_)).Times(0);

        auto& outputSocket = _lwipOutputSockets[i];
        EXPECT_CALL(outputSocket, bind(_, _)).Times(0);
        EXPECT_CALL(outputSocket, connect(_, _, _)).Times(0);
    }

    _pduTransportIntegration.activate(_ipAddress, _vlanId + 1, {});
    EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
    EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());
}

/**
 * \desc: Sending frames to active and disabled channels works.
 */
TEST_F(PduTransportIntegrationTest, send_frames_to_active_and_disabled_channels)
{
    constexpr size_t N = 16;

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        if (isTx(i))
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(3);
        }
        else
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(0);
        }
    }

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();
    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.disable();

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();
}

/**
 * \desc: Sending frames to active and disabled channels works with a smaller number of channels.
 */
TEST_F(
    PduTransportIntegrationTest,
    send_frames_to_active_and_disabled_channels_smaller_number_of_channels)
{
    constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = ::routing::NUM_PDU_TRANSPORT_CHANNELS - 1;
    constexpr size_t N                           = 16;

    new (&_pduTransportIntegration) PduTransportIntegration();

    _pduTransportIntegration.init(
        ::blob::CONFIGURATION_BLOB,
        ::etl::make_span(_pduTransportChannelIds).first(NUM_PDU_TRANSPORT_CHANNELS),
        ::etl::make_span(_inputSockets).first(NUM_PDU_TRANSPORT_CHANNELS),
        ::etl::make_span(_outputSockets).first(NUM_PDU_TRANSPORT_CHANNELS));

    for (size_t i = 0; i < NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        if (isTx(i))
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(3);
        }
        else
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(0);
        }
    }

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();
    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.disable();

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();
}

/**
 * \desc: Sending frames to active and disabled channels works with a bigger number of channels.
 */
TEST_F(
    PduTransportIntegrationTest,
    send_frames_to_active_and_disabled_channels_bigger_number_of_channels)
{
    constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = ::routing::NUM_PDU_TRANSPORT_CHANNELS + 1;
    constexpr size_t N                           = 16;

    ::etl::array<uint8_t, NUM_PDU_TRANSPORT_CHANNELS> pduTransportChannelIds;
    ::etl::array<::udp::AbstractDatagramSocket*, NUM_PDU_TRANSPORT_CHANNELS> inputSockets;
    ::etl::array<::udp::AbstractDatagramSocket*, NUM_PDU_TRANSPORT_CHANNELS> outputSockets;
    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        pduTransportChannelIds[i] = _pduTransportChannelIds[i];
        inputSockets[i]           = _inputSockets[i];
        outputSockets[i]          = _outputSockets[i];
    }
    pduTransportChannelIds.back() = _pduTransportChannelIds.back() + 1;
    inputSockets.back()           = nullptr;
    outputSockets.back()          = nullptr;

    new (&_pduTransportIntegration) PduTransportIntegration();

    _pduTransportIntegration.init(
        ::blob::CONFIGURATION_BLOB, pduTransportChannelIds, inputSockets, outputSockets);

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        if (isTx(i))
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(3);
        }
        else
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(0);
        }
    }

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();
    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.disable();

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    writeToAll(N);
    _pduTransportIntegration.sendUdpFrames();
}

/**
 * \desc: Initialization fails if there is any invalid socket.
 */
TEST_F(PduTransportIntegrationTest, init_invalid_sockets)
{
    new (&_pduTransportIntegration) PduTransportIntegration();

    auto inputSockets    = _inputSockets;
    inputSockets.front() = nullptr;

    _pduTransportIntegration.init(
        ::blob::CONFIGURATION_BLOB, _pduTransportChannelIds, inputSockets, _outputSockets);

    auto outputSockets    = _outputSockets;
    outputSockets.front() = nullptr;

    _pduTransportIntegration.init(
        ::blob::CONFIGURATION_BLOB, _pduTransportChannelIds, _inputSockets, outputSockets);

    EXPECT_EQ(0, _pduTransportIntegration.bufferedOutputWriters().size());
}

/**
 * \desc: A channel is not activated or disabled if its sockets are invalid.
 */
TEST_F(PduTransportIntegrationTest, activate_and_disable_invalid_sockets)
{
    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto& inputSocket = _lwipInputSockets[i];
        EXPECT_CALL(inputSocket, bind(_, _)).Times(0);
        EXPECT_CALL(inputSocket, join(_)).Times(0);
        EXPECT_CALL(inputSocket, close()).Times(0);

        auto& outputSocket = _lwipOutputSockets[i];
        EXPECT_CALL(outputSocket, bind(_, _)).Times(0);
        EXPECT_CALL(outputSocket, connect(_, _, _)).Times(0);
        EXPECT_CALL(outputSocket, close()).Times(0);
        EXPECT_CALL(outputSocket, disconnect()).Times(0);
    }

    _inputSockets.fill(nullptr);
    _outputSockets.fill(nullptr);

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});
    EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
    EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());

    _pduTransportIntegration.disable();
    EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
    EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());
}

/**
 * \desc: Data received on output sockets is discarded promptly.
 */
TEST_F(PduTransportIntegrationTest, discarding_listener)
{
    constexpr size_t LENGTH = 8;

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        if (isTx(i))
        {
            auto& socket = _lwipOutputSockets[i];
            EXPECT_CALL(socket, read(nullptr, LENGTH)).Times(1);

            socket.getDataListener()->dataReceived(socket, _ipAddress, 0, _ipAddress, LENGTH);
        }
    }
}

/**
 * \desc: Checking transmission timeout of channels works.
 */
TEST_F(PduTransportIntegrationTest, check_transmission_timeouts)
{
    constexpr size_t N   = 16;
    constexpr uint32_t T = 10;

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        if (isTx(i))
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(_transmissionTimeouts[i] == 0 ? (T + 1) : (T / _transmissionTimeouts[i]));
        }
        else
        {
            EXPECT_CALL(_lwipOutputSockets[i], send(Matcher<::etl::span<uint8_t const> const&>(_)))
                .Times(0);
        }
    }

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});

    commitToAll(N);
    for (uint32_t i = 0; i < T; ++i, _timestamp = 1000U * i)
    {
        _pduTransportIntegration.checkTransmissionTimeouts();
        _pduTransportIntegration.sendUdpFrames();
        commitToAll(N);
    }
    _pduTransportIntegration.checkTransmissionTimeouts();
    _pduTransportIntegration.sendUdpFrames();
}

/**
 * \desc: Calling methods without initialization does nothing.
 */
TEST_F(PduTransportIntegrationTest, methods_without_init)
{
    new (&_pduTransportIntegration) PduTransportIntegration();

    for (size_t i = 0; i < ::routing::NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto& inputSocket = _lwipInputSockets[i];
        EXPECT_CALL(inputSocket, bind(_, _)).Times(0);
        EXPECT_CALL(inputSocket, join(_)).Times(0);

        EXPECT_CALL(inputSocket, close()).Times(0);

        auto& outputSocket = _lwipOutputSockets[i];
        EXPECT_CALL(outputSocket, bind(_, _)).Times(0);
        EXPECT_CALL(outputSocket, connect(_, _, _)).Times(0);

        EXPECT_CALL(outputSocket, send(Matcher<::etl::span<uint8_t const> const&>(_))).Times(0);

        EXPECT_CALL(outputSocket, close()).Times(0);
        EXPECT_CALL(outputSocket, disconnect()).Times(0);
    }

    _pduTransportIntegration.activate(_ipAddress, _vlanId, {});
    EXPECT_EQ(0, _pduTransportIntegration.udpReceivers().size());
    EXPECT_EQ(0, _pduTransportIntegration.udpSenders().size());

    _pduTransportIntegration.checkTransmissionTimeouts();
    _pduTransportIntegration.sendUdpFrames();

    _pduTransportIntegration.disable();
}

} // namespace
