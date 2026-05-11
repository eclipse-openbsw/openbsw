/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/algorithm.h>
#include <etl/vector.h>
#include <io/IReader.h>
#include <io/IWriter.h>
#include <io/MemoryQueue.h>
#include <io/udp/Receiver.h>
#include <io/udp/Sender.h>
#include <lwipSocket/udp/LwipDatagramSocket.h>
#include <routing/ErrorHandler.h>
#include <routing/Logger.h>
#include <routing/PduTransportBufferedWriter.h>
#include <routing/PduTransportConfig.h>
#include <routing/channelId.h>
#include <routing/util.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <platform/estdint.h>

namespace routing
{
namespace logger = ::util::logger;

template<uint8_t MAX_NUM_CHANNELS, size_t MAX_ELEMENT_SIZE>
class PduTransportIntegration
{
public:
    using SIZE_TYPE                  = uint16_t;
    // Queue should always be able to store at least 2 elements. Because of the worst case scenario
    // of the memory queue, this requires 2 extra elements of MAX_ELEMENT_SIZE worth of space.
    static constexpr size_t CAPACITY = 4 * (MAX_ELEMENT_SIZE + sizeof(SIZE_TYPE));
    using Queue                      = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE, SIZE_TYPE>;
    using Reader                     = ::io::MemoryQueueReader<Queue>;
    using Writer                     = ::io::MemoryQueueWriter<Queue>;

    PduTransportIntegration()
    : _initialized(false)
    , _inputSockets()
    , _outputSockets()
    , _pduTransportChannelConfigs()
    , _udpReceivers()
    , _udpSenders()
    , _udpListeners()
    , _rxQueues()
    , _txQueues()
    , _rxReaders()
    , _txReaders()
    , _rxWriters()
    , _txWriters()
    , _pduTransportBufferedTxWriters()
    {
        for (size_t i = 0; i < MAX_NUM_CHANNELS; ++i)
        {
            auto& rxQueue = _rxQueues.emplace_back();
            (void)_rxReaders.emplace_back(rxQueue);
            (void)_rxWriters.emplace_back(rxQueue);

            auto& txQueue = _txQueues.emplace_back();
            (void)_txReaders.emplace_back(txQueue);
            (void)_txWriters.emplace_back(txQueue);
        }
    }

    void init(
        ::etl::span<uint8_t const> const blob,
        ::etl::span<uint8_t const> channelIds,
        ::etl::span<::udp::AbstractDatagramSocket* const> inputSockets,
        ::etl::span<::udp::AbstractDatagramSocket* const> outputSockets)
    {
        if (_initialized)
        {
            logger::Logger::error(logger::ROUTING, "Already initialized");
            return;
        }

        size_t const numberOfChannels = ::etl::min(
            static_cast<size_t>(MAX_NUM_CHANNELS),
            ::etl::min(channelIds.size(), ::etl::min(inputSockets.size(), outputSockets.size())));
        channelIds    = channelIds.first(numberOfChannels);
        inputSockets  = inputSockets.first(numberOfChannels);
        outputSockets = outputSockets.first(numberOfChannels);

        if ((::etl::find(inputSockets.begin(), inputSockets.end(), nullptr) != inputSockets.end())
            || (::etl::find(outputSockets.begin(), outputSockets.end(), nullptr)
                != outputSockets.end()))
        {
            logger::Logger::error(
                logger::ROUTING, "Initialization failed because of invalid socket(s)");
            return;
        }

        _inputSockets  = inputSockets;
        _outputSockets = outputSockets;

        for (size_t i = 0; i < numberOfChannels; ++i)
        {
            auto const channelConfig
                = ::routing::config(blob, ::blob::Config::Type::CHANNEL, channelIds[i]);
            auto const channelConfigData
                = _pduTransportChannelConfigs.emplace_back(channelConfig.data);

            ::routing::PduTransportConfig pduTransportConfig;
            if (!::routing::load(channelConfigData, pduTransportConfig))
            {
                logger::Logger::error(
                    logger::ROUTING, "Could not load PDU transport channel %d config", i);
            }

            (void)_pduTransportBufferedTxWriters.emplace_back(
                _txWriters[i], pduTransportConfig.transmissionTimeout);
        }

        _initialized = true;
    }

    ::etl::span<Queue const> inputQueues() const { return _rxQueues; }

    ::etl::span<Queue const> outputQueues() const { return _txQueues; }

    ::etl::span<Reader> inputReaders() { return _rxReaders; }

    ::etl::span<Writer> inputWriters() { return _rxWriters; }

    ::etl::span<Writer> outputWriters() { return _txWriters; }

    ::etl::span<::routing::PduTransportBufferedWriter> bufferedOutputWriters()
    {
        return _pduTransportBufferedTxWriters;
    }

    ::etl::span<::io::udp::Receiver const> udpReceivers() const { return _udpReceivers; }

    ::etl::span<::io::udp::Sender const> udpSenders() const { return _udpSenders; }

    void checkTransmissionTimeouts()
    {
        if (!_initialized)
        {
            logger::Logger::error(logger::ROUTING, "Uninitialized");
            return;
        }

        for (auto& writer : _pduTransportBufferedTxWriters)
        {
            writer.checkTransmissionTimeout();
        }
    }

    void sendUdpFrames()
    {
        if (!_initialized)
        {
            logger::Logger::error(logger::ROUTING, "Uninitialized");
            return;
        }

        constexpr size_t MAX_NUM_FRAMES = 10;
        for (auto& udpSender : _udpSenders)
        {
            udpSender.run(MAX_NUM_FRAMES);
        }
    }

    void activate(
        ::ip::IPAddress const& localIpAddress,
        uint16_t const vlanId,
        ::routing::ErrorHandler::Function const errorHandlingFunction)
    {
        if (!_initialized)
        {
            logger::Logger::error(logger::ROUTING, "Uninitialized");
            return;
        }

        for (size_t i = 0; i < _pduTransportChannelConfigs.size(); ++i)
        {
            ::routing::PduTransportConfig pduTransportConfig;
            if (!::routing::load(_pduTransportChannelConfigs[i], pduTransportConfig))
            {
                logger::Logger::error(
                    logger::ROUTING,
                    "Could not activate pduTransport channel %d due to invalid config",
                    i);
                continue;
            }

            if (pduTransportConfig.vlanId != vlanId)
            {
                continue;
            }

            clear(_rxReaders[i]);
            clear(_txReaders[i]);
            _pduTransportBufferedTxWriters[i].reset();

            bool const isRxTx
                = pduTransportConfig.mode == ::routing::PduTransportConfig::Mode::RX_TX;
            bool const isRx
                = isRxTx || (pduTransportConfig.mode == ::routing::PduTransportConfig::Mode::RX);
            bool const isTx
                = isRxTx || (pduTransportConfig.mode == ::routing::PduTransportConfig::Mode::TX);

            if (isRx)
            {
                enableRx(i, pduTransportConfig, localIpAddress, errorHandlingFunction);
            }

            if (isTx)
            {
                enableTx(i, pduTransportConfig, localIpAddress);
            }
        }
    }

    void disable()
    {
        if (!_initialized)
        {
            logger::Logger::error(logger::ROUTING, "Uninitialized");
            return;
        }

        for (size_t i = 0; i < _pduTransportChannelConfigs.size(); ++i)
        {
            if (_outputSockets[i] != nullptr)
            {
                _outputSockets[i]->disconnect();
                _outputSockets[i]->close();
            }

            if (_inputSockets[i] != nullptr)
            {
                _inputSockets[i]->close();
            }
        }

        _udpListeners.clear();
        _udpSenders.clear();
        _udpReceivers.clear();
    }

private:
    class DiscardingListener : private ::udp::IDataListener
    {
    public:
        explicit DiscardingListener(::udp::AbstractDatagramSocket& socket) : _socket(socket)
        {
            _socket.setDataListener(this);
        }

        virtual ~DiscardingListener() {}

    private:
        void dataReceived(
            ::udp::AbstractDatagramSocket& /* socket */,
            ::ip::IPAddress /*sourceAddress*/,
            uint16_t const /*sourcePort*/,
            ::ip::IPAddress /*destinationAddress*/,
            uint16_t const length) override
        {
            (void)_socket.read(nullptr, length);
        }

        ::udp::AbstractDatagramSocket& _socket;
    };

    static void clear(::io::IReader& reader)
    {
        while (!reader.peek().empty())
        {
            reader.release();
        }
    }

    void enableRx(
        size_t const index,
        ::routing::PduTransportConfig const& pduTransportConfig,
        ::ip::IPAddress const& localIpAddress,
        ::routing::ErrorHandler::Function const errorHandlingFunction)
    {
        auto const inputSocket = _inputSockets[index];

        if (inputSocket == nullptr)
        {
            logger::Logger::error(logger::ROUTING, "Invalid input socket %d", index);
            return;
        }

        (void)_udpReceivers.emplace_back(
            *inputSocket,
            _rxWriters[index],
            ::routing::ErrorHandler(errorHandlingFunction, pduTransportConfig.header.channelId),
            pduTransportConfig.remoteIpAddresses);

        bool const isBound = inputSocket->bind(&localIpAddress, pduTransportConfig.remotePort)
                             == udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
        if (!isBound)
        {
            logger::Logger::error(logger::ROUTING, "Could not bind input socket %d", index);
            return;
        }

        bool const isJoined = inputSocket->join(pduTransportConfig.ipAddress)
                              == udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
        if (!isJoined)
        {
            logger::Logger::error(
                logger::ROUTING, "Could not join socket %d to multicast address", index);
            return;
        }
    }

    void enableTx(
        size_t const index,
        ::routing::PduTransportConfig const& pduTransportConfig,
        ::ip::IPAddress const& localIpAddress)
    {
        auto const outputSocket = _outputSockets[index];

        if (outputSocket == nullptr)
        {
            logger::Logger::error(logger::ROUTING, "Invalid output socket %d", index);
            return;
        }

        (void)_udpListeners.emplace_back(*outputSocket);

        bool const isBound = outputSocket->bind(&localIpAddress, pduTransportConfig.localPort)
                             == udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;
        if (!isBound)
        {
            logger::Logger::error(logger::ROUTING, "Could not bind output socket %d", index);
            return;
        }

        bool const isConnected = outputSocket->connect(
                                     pduTransportConfig.ipAddress,
                                     pduTransportConfig.remotePort,
                                     const_cast<::ip::IPAddress*>(&localIpAddress))
                                 == udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK;

        if (!isConnected)
        {
            logger::Logger::error(
                logger::ROUTING, "Could not connect output socket %d to multicast address.", index);
            return;
        }

        (void)_udpSenders.emplace_back(_txReaders[index], *outputSocket);
    }

    bool _initialized;

    ::etl::span<::udp::AbstractDatagramSocket* const> _inputSockets;
    ::etl::span<::udp::AbstractDatagramSocket* const> _outputSockets;

    ::etl::vector<::etl::span<uint8_t const>, MAX_NUM_CHANNELS> _pduTransportChannelConfigs;

    ::etl::vector<::io::udp::Receiver, MAX_NUM_CHANNELS> _udpReceivers;
    ::etl::vector<::io::udp::Sender, MAX_NUM_CHANNELS> _udpSenders;
    ::etl::vector<DiscardingListener, MAX_NUM_CHANNELS> _udpListeners;

    ::etl::vector<Queue, MAX_NUM_CHANNELS> _rxQueues;
    ::etl::vector<Queue, MAX_NUM_CHANNELS> _txQueues;
    ::etl::vector<Reader, MAX_NUM_CHANNELS> _rxReaders;
    ::etl::vector<Reader, MAX_NUM_CHANNELS> _txReaders;
    ::etl::vector<Writer, MAX_NUM_CHANNELS> _rxWriters;
    ::etl::vector<Writer, MAX_NUM_CHANNELS> _txWriters;
    ::etl::vector<::routing::PduTransportBufferedWriter, MAX_NUM_CHANNELS>
        _pduTransportBufferedTxWriters;
};

} // namespace routing
