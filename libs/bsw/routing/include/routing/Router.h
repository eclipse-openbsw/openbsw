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

#include "routing/Logger.h"
#include "routing/PduRoutingTable.h"
#include "routing/pduRouting.h"

#include <etl/algorithm.h>
#include <etl/span.h>
#include <io/IReader.h>
#include <io/IWriter.h>

#include <platform/estdint.h>

namespace routing
{
/**
 * A PDU Router that forwards PDUs from readers to writers according to its routing table.
 *
 * [TPARAMS_BEGIN]
 * \tparam MAX_NUM_CHANNELS The maximum number of channels to write to and from. Needs to fit the
 * routing table data. [TPARAMS_END]
 */
template<uint8_t MAX_NUM_CHANNELS>
class Router
{
public:
    // [PUBLIC_API_BEGIN]
    /**
     * Construct an uninitialized Router.
     */
    Router() = default;

    /**
     * Initialize a Router using a pre-filled PduRoutingTable.
     *
     * It starts with transmission and reception enabled for all channels.
     */
    void init(
        ::routing::PduRoutingTable const& table,
        ::etl::span<::io::IReader*> readers,
        ::etl::span<::io::IWriter*> writers);

    /**
     * Initialize a Router by loading the PduRoutingTable from the provided config byte array.
     *
     * If the config data is invalid, the Router will be uninitialized and the other functions will
     * not do anything apart from logging an error.
     *
     * Otherwise, it starts with transmission and reception enabled for all channels.
     */
    void init(
        ::etl::span<uint8_t const> config,
        ::etl::span<::io::IReader*> readers,
        ::etl::span<::io::IWriter*> writers);

    /**
     * Try routing a PDU from a channel.
     *
     * It returns true once a PDU is routed. Otherwise, it returns false after attempting once from
     * each channel.
     */
    bool run();

    // [PUBLIC_API_END]
private:
    ::routing::PduRoutingTable _table;
    ::etl::span<::io::IReader*> _readers;
    ::etl::span<::io::IWriter*> _writers;
    bool _initialized         = false;
    uint8_t _currentChannelId = 0;
};

template<uint8_t MAX_NUM_CHANNELS>
void Router<MAX_NUM_CHANNELS>::init(
    ::routing::PduRoutingTable const& table,
    ::etl::span<::io::IReader*> readers,
    ::etl::span<::io::IWriter*> writers)
{
    if (_initialized)
    {
        ::util::logger::Logger::warn(::util::logger::ROUTING, "Router already initialized");
        return;
    }

    size_t const numberOfChannels = ::etl::min(
        static_cast<size_t>(MAX_NUM_CHANNELS), ::etl::min(readers.size(), writers.size()));
    readers = readers.first(numberOfChannels);
    writers = writers.first(numberOfChannels);

    if ((::etl::find(readers.begin(), readers.end(), nullptr) != readers.end())
        || (::etl::find(writers.begin(), writers.end(), nullptr) != writers.end()))
    {
        ::util::logger::Logger::warn(
            ::util::logger::ROUTING, "Invalid reader(s) or writer(s) at initialization");
    }

    if ((table.destinations.empty()) || (table.destinationOffsets.empty())
        || (table.outputMessageIds.empty()))
    {
        ::util::logger::Logger::error(
            ::util::logger::ROUTING, "Failed to initialize router: empty routing configuration");
        return;
    }

    if (table.destinations.size() != table.outputMessageIds.size())
    {
        ::util::logger::Logger::error(
            ::util::logger::ROUTING,
            "Failed to initialize router: destinations and output message IDs have different "
            "sizes");
        return;
    }

    _table   = table;
    _readers = readers;
    _writers = writers;

    _initialized = true;
}

template<uint8_t MAX_NUM_CHANNELS>
void Router<MAX_NUM_CHANNELS>::init(
    ::etl::span<uint8_t const> const config,
    ::etl::span<::io::IReader*> const readers,
    ::etl::span<::io::IWriter*> const writers)
{
    if (_initialized)
    {
        ::util::logger::Logger::warn(::util::logger::ROUTING, "Router already initialized");
        return;
    }

    if (!load(config, _table))
    {
        ::util::logger::Logger::error(
            ::util::logger::ROUTING, "Failed to load routing table from config");
    }
    init(_table, readers, writers);
}

template<uint8_t MAX_NUM_CHANNELS>
bool Router<MAX_NUM_CHANNELS>::run()
{
    if (!_initialized)
    {
        ::util::logger::Logger::error(::util::logger::ROUTING, "Router uninitialized");
        return false;
    }

    for (size_t i = 0; i < _readers.size(); ++i)
    {
        auto const reader = _readers[_currentChannelId];
        bool const pduRouted
            = reader != nullptr ? route(_currentChannelId, _table, *reader, _writers) : false;
        _currentChannelId = static_cast<size_t>(_currentChannelId) + 1U < _readers.size()
                                ? _currentChannelId + 1U
                                : 0U;
        if (pduRouted)
        {
            return true;
        }
    }

    return false;
}

} // namespace routing
