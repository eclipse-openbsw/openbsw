/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <ip/IPAddress.h>

#include <etl/flat_map.h>
#include <cstdint>

namespace someip
{
namespace internal
{
uint16_t const INITIAL_SESSION_ID = 1U;
uint16_t const MAX_SESSION_ID     = 0xFFFFU;

class SessionEntry
{
public:
    SessionEntry();
    void clear();
    void increment();

    uint16_t _sessionId;
    bool _rebootFlag;
};

class SessionEndpoint : public SessionEntry
{
public:
    SessionEndpoint();
    void clear();
};

} // namespace internal

class SessionManager
{
public:
    void init();

    void getSessionInfoForNextMulticastMessage(uint16_t& sessionId, bool& rebootFlag);

    void getSessionInfoForNextUnicastMessage(
        ::ip::IPAddress const& ipAddress, uint16_t& sessionId, bool& rebootFlag);

protected:
    using EndpointMap
        = ::etl::iflat_map<::ip::IPAddress, internal::SessionEndpoint, ::ip::IPAddressCompareLess>;

    explicit SessionManager(EndpointMap& map);

private:
    EndpointMap::iterator addEndpoint(::ip::IPAddress const& ipAddress);

    EndpointMap& _endpoints;
    internal::SessionEntry _multicast;
};

namespace declare
{
template<uint16_t NUM_ENDPOINTS>
class SessionManager : public ::someip::SessionManager
{
public:
    SessionManager();

private:
    ::etl::flat_map<
        ::ip::IPAddress,
        internal::SessionEndpoint,
        NUM_ENDPOINTS,
        ::ip::IPAddressCompareLess>
        _endpointMap;
};

template<uint16_t NUM_ENDPOINTS>
inline SessionManager<NUM_ENDPOINTS>::SessionManager()
: ::someip::SessionManager(_endpointMap), _endpointMap()
{}

} // namespace declare
} // namespace someip
