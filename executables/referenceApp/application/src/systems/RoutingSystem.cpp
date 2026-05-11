/********************************************************************************
 * Copyright (c) 2022 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/RoutingSystem.h"

#include <blob/configuration.h>
#include <routing/Logger.h>
#include <ethConfig.h>

#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/vector.h>

namespace systems
{
namespace logger = ::util::logger;

RoutingSystem::RoutingSystem(::async::ContextType const context, ::can::ICanSystem& canSystem)
: ::etl::singleton_base<RoutingSystem>{*this}
, _initialized(false)
, _pduTransportIntegration()
, _integration()
, _timeout()
, _context(context)
, _canSystem(canSystem)
, _errorHandlingFunction(
      decltype(_errorHandlingFunction)::create<RoutingSystem, &RoutingSystem::handleError>(*this))
{}

void RoutingSystem::init()
{
    ::etl::span<uint8_t const> const blob = ::blob::CONFIGURATION_BLOB;

    if (blob.empty())
    {
        logger::Logger::error(logger::ROUTING, "Blob is an empty slice");

        transitionDone();

        return;
    }

    ::etl::vector<uint8_t, MAX_NUM_PDU_TRANSPORT_CHANNELS> pduTransportChannelIds;
    ::etl::vector<::io::IReader*, MAX_NUM_PDU_TRANSPORT_CHANNELS> pduTransportReaders;
    ::etl::vector<::io::IWriter*, MAX_NUM_PDU_TRANSPORT_CHANNELS> pduTransportWriters;

    for (uint8_t i = 0; i < MAX_NUM_PDU_TRANSPORT_CHANNELS; ++i)
    {
        auto const pduTransportChannelId = Integration::FIRST_PDU_TRANSPORT_CHANNEL_ID + i;
        auto const channelConfig
            = ::routing::config(blob, ::blob::ConfigType::CHANNEL, pduTransportChannelId);
        if (channelConfig == ::blob::Config())
        {
            break;
        }

        pduTransportChannelIds.emplace_back(pduTransportChannelId);
        _inputSockets.emplace_back(&_lwipInputSockets.emplace_back());
        _outputSockets.emplace_back(&_lwipOutputSockets.emplace_back());
    }

    _pduTransportIntegration.init(blob, pduTransportChannelIds, _inputSockets, _outputSockets);

    auto const inputReaders          = _pduTransportIntegration.inputReaders();
    auto const bufferedOutputWriters = _pduTransportIntegration.bufferedOutputWriters();

    for (size_t i = 0; i < pduTransportChannelIds.size(); ++i)
    {
        pduTransportReaders.emplace_back(&inputReaders[i]);
        pduTransportWriters.emplace_back(&bufferedOutputWriters[i]);
    }

    ::etl::array<::io::IReader*, NUM_CAN_CHANNELS> canReaders;
    ::etl::array<::io::IWriter*, NUM_CAN_CHANNELS> canWriters;

    auto& queue       = _canInputQueues.emplace_back();
    auto& reader      = _canInputReaders.emplace_back(queue);
    auto& inputWriter = _canInputWriters.emplace_back(queue);
    (void)_canRxListeners.emplace_back(inputWriter);
    auto& outputWriter
        = _canOutputWriters.emplace_back(*_canSystem.getCanTransceiver(::busid::CAN_0));

    canReaders.front() = &reader;
    canWriters.front() = &outputWriter;

    _integration.init(
        blob,
        pduTransportReaders,
        pduTransportWriters,
        ::routing::canChannelIds,
        canReaders,
        canWriters,
        {},
        {},
        {},
        _errorHandlingFunction);

    _initialized = true;

    transitionDone();
}

void RoutingSystem::run()
{
    if (!_initialized)
    {
        transitionDone();

        return;
    }

    _canSystem.getCanTransceiver(::busid::CAN_0)->addCANFrameListener(_canRxListeners.front());

    _pduTransportIntegration.activate(::eth1::IP_ADDRESS, 160, _errorHandlingFunction);

    ::async::scheduleAtFixedRate(_context, *this, _timeout, 1U, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void RoutingSystem::shutdown()
{
    if (!_initialized)
    {
        transitionDone();

        return;
    }

    _timeout.cancel();

    _pduTransportIntegration.disable();

    _canSystem.getCanTransceiver(::busid::CAN_0)->removeCANFrameListener(_canRxListeners.front());

    transitionDone();
}

void RoutingSystem::execute()
{
    // PduTransportIntegration must run in the Ethernet context because socket API is not
    // thread-safe.
    _pduTransportIntegration.checkTransmissionTimeouts();
    _pduTransportIntegration.sendUdpFrames();

    _integration.route();
}

void RoutingSystem::handleError(
    ::routing::ErrorHandler::StatusCode const statusCode,
    uint8_t const /*channelId*/,
    uint32_t const /*data*/)
{
    switch (statusCode)
    {
        case ::routing::ErrorHandler::StatusCode::INVALID_MESSAGE_LENGTH:
            logger::Logger::debug(logger::ROUTING, "Invalid message length");
            break;
        case ::routing::ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE:
            logger::Logger::debug(logger::ROUTING, "Memory allocation failure");
            break;
        case ::routing::ErrorHandler::StatusCode::INVALID_MESSAGE_HEADER_SIZE:
            logger::Logger::debug(logger::ROUTING, "Invalid message header size");
            break;
        case ::routing::ErrorHandler::StatusCode::INVALID_PARSED_LENGTH:
            logger::Logger::debug(logger::ROUTING, "Invalid parsed length");
            break;
        case ::routing::ErrorHandler::StatusCode::INVALID_REMOTE_IP_ADDRESS:
            logger::Logger::debug(logger::ROUTING, "Invalid remote IP address");
            break;
        case ::routing::ErrorHandler::StatusCode::UNKNOWN_MESSAGE_ID:
            logger::Logger::debug(logger::ROUTING, "Unknown message ID");
            break;
        case ::routing::ErrorHandler::StatusCode::INVALID_PAYLOAD_LENGTH:
            logger::Logger::debug(logger::ROUTING, "Invalid payload length");
            break;
        default: logger::Logger::debug(logger::ROUTING, "Unknown error"); break;
    }
}

} // namespace systems
