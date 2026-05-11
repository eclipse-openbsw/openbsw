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

#include "io/udp/Receiver.h"
#include "io/udp/Sender.h"
#include "routing/ChannelNames.h"
#include "routing/ErrorHandler.h"
#include "routing/Header.h"
#include "routing/LegacyRxAdapter.h"
#include "routing/LegacyTxAdapter.h"
#include "routing/Logger.h"
#include "routing/PduTransportConfig.h"
#include "routing/PduTransportRxAdapter.h"
#include "routing/PduTransportTxAdapter.h"
#include "routing/Router.h"
#include "routing/RxAdapter.h"
#include "routing/RxAdapterTable.h"
#include "routing/TxAdapterTable.h"
#include "routing/util.h"

#include <blob/Blob.h>
#include <blob/Config.h>
#include <blob/util.h>
#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/optional.h>
#include <etl/span.h>
#include <io/BufferedWriter.h>
#include <io/IReader.h>
#include <io/IWriter.h>
#include <lwipSocket/udp/LwipDatagramSocket.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <platform/estdint.h>

namespace routing
{
namespace logger = ::util::logger;

template<
    uint8_t MAX_NUM_PDU_TRANSPORT_CHANNELS,
    uint8_t NUM_CAN_CHANNELS,
    uint8_t NUM_FLEXRAY_CHANNELS,
    size_t MAX_PDU_TRANSPORT_CHANNEL_ELEMENT_SIZE,
    size_t MAX_CAN_CHANNEL_ELEMENT_SIZE,
    size_t MAX_FLEXRAY_CHANNEL_ELEMENT_SIZE>
class Integration
{
public:
    static_assert(
        MAX_NUM_PDU_TRANSPORT_CHANNELS + NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS > 0,
        "Number of channels must be greater than 0");

    static constexpr uint8_t MAX_NUM_CHANNELS
        = MAX_NUM_PDU_TRANSPORT_CHANNELS + NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS;
    static constexpr uint8_t FIRST_PDU_TRANSPORT_CHANNEL_ID
        = MAX_NUM_CHANNELS - MAX_NUM_PDU_TRANSPORT_CHANNELS;

    template<typename RxAdapterType, typename TxAdapterType>
    struct ChannelAdapter
    {
        ChannelAdapter() = default;

        uint8_t channelId        = INVALID_CHANNEL_ID;
        RxAdapterType* rxAdapter = nullptr;
        TxAdapterType* txAdapter = nullptr;
    };

    using PduTransportRxAdapter
        = ::routing::PduTransportRxAdapter<MAX_PDU_TRANSPORT_CHANNEL_ELEMENT_SIZE>;
    using CanRxAdapter     = ::routing::LegacyRxAdapter<MAX_CAN_CHANNEL_ELEMENT_SIZE>;
    using FlexrayRxAdapter = ::routing::LegacyRxAdapter<MAX_FLEXRAY_CHANNEL_ELEMENT_SIZE>;

    using PduTransportChannelAdapter = ChannelAdapter<PduTransportRxAdapter, PduTransportTxAdapter>;
    using CanChannelAdapter          = ChannelAdapter<CanRxAdapter, LegacyTxAdapter>;
    using FlexrayChannelAdapter      = ChannelAdapter<FlexrayRxAdapter, LegacyTxAdapter>;

    Integration()
    : _initialized(false)
    , _numberOfPduTransportChannels(0U)
    , _numberOfCanChannels(0U)
    , _numberOfFlexrayChannels(0U)
    , _pduTransportRxAdapters()
    , _pduTransportTxAdapters()
    , _pduTransportChannelAdapters()
    , _canRxAdapters()
    , _canTxAdapters()
    , _canChannelAdapters()
    , _flexrayRxAdapters()
    , _flexrayTxAdapters()
    , _flexrayChannelAdapters()
    , _readers()
    , _writers()
    , _router()
    {
        _readers.fill(nullptr);
        _writers.fill(nullptr);
    }

    void init(
        ::etl::span<uint8_t const> const blobData,
        ::etl::span<::io::IReader*> const pduTransportReaders,
        ::etl::span<::io::IWriter*> const pduTransportWriters,
        ::etl::span<uint8_t const, NUM_CAN_CHANNELS> const canChannelIds,
        ::etl::span<::io::IReader*, NUM_CAN_CHANNELS> const canReaders,
        ::etl::span<::io::IWriter*, NUM_CAN_CHANNELS> const canWriters,
        ::etl::span<uint8_t const, NUM_FLEXRAY_CHANNELS> const frChannelIds,
        ::etl::span<::io::IReader*, NUM_FLEXRAY_CHANNELS> const flexrayReaders,
        ::etl::span<::io::IWriter*, NUM_FLEXRAY_CHANNELS> const flexrayWriters,
        ::routing::ErrorHandler::Function const errorHandlingFunction)
    {
        if (_initialized)
        {
            return;
        }

        if (blobData.empty())
        {
            logger::Logger::error(logger::ROUTING, "Blob is an empty span");

            return;
        }

        size_t const maxNumberOfPduTransportChannels = ::etl::min(
            static_cast<size_t>(MAX_NUM_PDU_TRANSPORT_CHANNELS),
            ::etl::min(pduTransportReaders.size(), pduTransportWriters.size()));
        size_t const numberOfChannels
            = maxNumberOfPduTransportChannels + NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS;

        auto const readers = ::etl::make_span(_readers).first(numberOfChannels);
        auto const writers = ::etl::make_span(_writers).first(numberOfChannels);

        auto const routingTable = ::blob::config(blobData, ::blob::Config::Type::ROUTING).data;
        ::routing::ChannelNames channelNamesConfig;
        (void)::routing::load(
            ::routing::config(blobData, ::blob::Config::Type::CHANNEL_NAMES).data,
            channelNamesConfig);

        for (auto const config : ::blob::Blob(blobData))
        {
            if (config.type == ::blob::Config::Type::RX_ADAPTER)
            {
                auto data           = config.data;
                Header const header = load(data);

                auto const channelId = header.channelId;

                if ((channelId >= numberOfChannels) || (channelId == INVALID_CHANNEL_ID))
                {
                    logger::Logger::error(
                        logger::ROUTING, "Unable to add channel with invalid ID %d", channelId);
                    continue;
                }

                logger::Logger::debug(
                    logger::ROUTING,
                    "adding channel id %d of type %x\n",
                    static_cast<uint32_t>(channelId),
                    static_cast<uint32_t>(header.channelType));

                auto const channelName
                    = ::routing::name(channelNamesConfig, channelId).reinterpret_as<char const>();
                auto const name = channelName.empty() ? "" : channelName.data();

                switch (header.channelType)
                {
                    case ChannelType::CAN:
                    {
                        if (_numberOfCanChannels >= NUM_CAN_CHANNELS)
                        {
                            continue;
                        }

                        auto const txConfig = ::routing::config(
                            blobData, ::blob::Config::Type::TX_ADAPTER, channelId);
                        if (txConfig.type == ::blob::ConfigType::UNKNOWN)
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Couldn't find TX adapter config for %d\n",
                                static_cast<uint32_t>(channelId));
                            continue;
                        }

                        auto const it
                            = etl::find(canChannelIds.begin(), canChannelIds.end(), channelId);
                        if (it == canChannelIds.end())
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Channel ID %d not found while adding CAN channel",
                                channelId);
                            continue;
                        }

                        auto const index
                            = static_cast<size_t>(::etl::distance(canChannelIds.begin(), it));

                        if ((canReaders[index] == nullptr) || (canWriters[index] == nullptr))
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Unable to add CAN channel (id=%d, name=%s) due to invalid reader "
                                "or writer",
                                channelId,
                                name);
                            continue;
                        }

                        logger::Logger::debug(
                            logger::ROUTING,
                            "Adding CAN channel (id=%d, name=%s) to router",
                            channelId,
                            name);

                        auto reader = &_canRxAdapters[index].emplace(
                            *canReaders[index],
                            config.data,
                            ::routing::ErrorHandler(errorHandlingFunction, channelId));
                        auto writer = &_canTxAdapters[index].emplace(
                            *canWriters[index],
                            txConfig.data,
                            ::routing::ErrorHandler(errorHandlingFunction, channelId));

                        _canChannelAdapters[_numberOfCanChannels] = {channelId, reader, writer};

                        readers[channelId] = reader;
                        writers[channelId] = writer;

                        ++_numberOfCanChannels;
                    }
                    break;
                    case ChannelType::PDU_TRANSPORT:
                    {
                        if (_numberOfPduTransportChannels >= maxNumberOfPduTransportChannels)
                        {
                            continue;
                        }

                        if (channelId < FIRST_PDU_TRANSPORT_CHANNEL_ID)
                        {
                            logger::Logger::error(
                                logger::ROUTING, "Invalid pduTransport channel ID %d", channelId);
                            continue;
                        }

                        uint8_t const index = channelId - FIRST_PDU_TRANSPORT_CHANNEL_ID;

                        if ((pduTransportReaders[index] == nullptr)
                            || (pduTransportWriters[index] == nullptr))
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Unable to add pduTransport channel (id=%d, name=%s) due to "
                                "invalid reader "
                                "or writer",
                                channelId,
                                name);
                            continue;
                        }

                        logger::Logger::debug(
                            logger::ROUTING,
                            "Adding pduTransport channel (id=%d, name=%s) to router",
                            channelId,
                            name);

                        auto reader = &_pduTransportRxAdapters[index].emplace(
                            *pduTransportReaders[index],
                            config.data,
                            ::routing::ErrorHandler(errorHandlingFunction, channelId));
                        auto writer = &_pduTransportTxAdapters[index].emplace(
                            *pduTransportWriters[index],
                            ::routing::ErrorHandler(errorHandlingFunction, channelId));

                        _pduTransportChannelAdapters[_numberOfPduTransportChannels]
                            = {channelId, reader, writer};
                        readers[channelId] = reader;
                        writers[channelId] = writer;

                        ++_numberOfPduTransportChannels;
                    }
                    break;
                    case ChannelType::FR:
                    {
                        if (_numberOfFlexrayChannels >= NUM_FLEXRAY_CHANNELS)
                        {
                            continue;
                        }

                        auto const txConfig = ::routing::config(
                            blobData, ::blob::Config::Type::TX_ADAPTER, channelId);
                        if (txConfig.type == ::blob::ConfigType::UNKNOWN)
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Couldn't find TX adapter config for %d\n",
                                static_cast<uint32_t>(channelId));
                            continue;
                        }

                        auto const it
                            = etl::find(frChannelIds.begin(), frChannelIds.end(), channelId);
                        if (it == frChannelIds.end())
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Channel ID %d not found while adding FlexRay channel",
                                channelId);
                            continue;
                        }

                        auto const index
                            = static_cast<size_t>(::etl::distance(frChannelIds.begin(), it));

                        if ((flexrayReaders[index] == nullptr)
                            || (flexrayWriters[index] == nullptr))
                        {
                            logger::Logger::error(
                                logger::ROUTING,
                                "Unable to add FlexRay channel (id=%d, name=%s) due to invalid "
                                "reader or writer",
                                channelId,
                                name);
                            continue;
                        }

                        logger::Logger::debug(
                            logger::ROUTING,
                            "Adding FlexRay channel (id=%d, name=%s) to router",
                            channelId,
                            name);

                        auto reader = &_flexrayRxAdapters[index].emplace(
                            *flexrayReaders[index],
                            config.data,
                            ::routing::ErrorHandler(errorHandlingFunction, channelId));
                        auto writer = &_flexrayTxAdapters[index].emplace(
                            *flexrayWriters[index],
                            txConfig.data,
                            ::routing::ErrorHandler(errorHandlingFunction, channelId));

                        _flexrayChannelAdapters[_numberOfFlexrayChannels]
                            = {channelId, reader, writer};

                        readers[channelId] = reader;
                        writers[channelId] = writer;

                        ++_numberOfFlexrayChannels;
                    }
                    break;
                    default:
                    {
                        // unsupported channel type
                    }
                    break;
                }
            }
        }

        _router.init(routingTable, readers, writers);

        _initialized = true;
    }

    bool route()
    {
        if (!_initialized)
        {
            return false;
        }

        return _router.run();
    }

    ::etl::span<PduTransportChannelAdapter const> pduTransportChannelAdapters() const
    {
        if (!_initialized)
        {
            return {};
        }

        return ::etl::make_span(_pduTransportChannelAdapters).first(_numberOfPduTransportChannels);
    }

    ::etl::span<CanChannelAdapter const> canChannelAdapters() const
    {
        if (!_initialized)
        {
            return {};
        }

        return ::etl::make_span(_canChannelAdapters).first(_numberOfCanChannels);
    }

    ::etl::span<FlexrayChannelAdapter const> flexrayChannelAdapters() const
    {
        if (!_initialized)
        {
            return {};
        }

        return ::etl::make_span(_flexrayChannelAdapters).first(_numberOfFlexrayChannels);
    }

private:
    bool _initialized;

    uint8_t _numberOfPduTransportChannels;
    uint8_t _numberOfCanChannels;
    uint8_t _numberOfFlexrayChannels;

    ::etl::array<::etl::optional<PduTransportRxAdapter>, MAX_NUM_PDU_TRANSPORT_CHANNELS>
        _pduTransportRxAdapters;
    ::etl::array<::etl::optional<PduTransportTxAdapter>, MAX_NUM_PDU_TRANSPORT_CHANNELS>
        _pduTransportTxAdapters;
    ::etl::array<PduTransportChannelAdapter, MAX_NUM_PDU_TRANSPORT_CHANNELS>
        _pduTransportChannelAdapters;

    ::etl::array<::etl::optional<CanRxAdapter>, NUM_CAN_CHANNELS> _canRxAdapters;
    ::etl::array<::etl::optional<LegacyTxAdapter>, NUM_CAN_CHANNELS> _canTxAdapters;
    ::etl::array<CanChannelAdapter, NUM_CAN_CHANNELS> _canChannelAdapters;

    ::etl::array<::etl::optional<FlexrayRxAdapter>, NUM_FLEXRAY_CHANNELS> _flexrayRxAdapters;
    ::etl::array<::etl::optional<LegacyTxAdapter>, NUM_FLEXRAY_CHANNELS> _flexrayTxAdapters;
    ::etl::array<FlexrayChannelAdapter, NUM_FLEXRAY_CHANNELS> _flexrayChannelAdapters;

    ::etl::array<::io::IReader*, MAX_NUM_CHANNELS> _readers;
    ::etl::array<::io::IWriter*, MAX_NUM_CHANNELS> _writers;

    Router<MAX_NUM_CHANNELS> _router;
};

} // namespace routing
