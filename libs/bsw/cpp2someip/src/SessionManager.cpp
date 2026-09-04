/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SessionManager.h"

#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPAddress;
using ::util::logger::SOMEIP;

namespace internal
{
SessionEntry::SessionEntry() : _sessionId(INITIAL_SESSION_ID), _rebootFlag(true) {}

void SessionEntry::clear()
{
    _sessionId  = INITIAL_SESSION_ID;
    _rebootFlag = true;
}

void SessionEntry::increment()
{
    if (_sessionId == MAX_SESSION_ID)
    {
        _sessionId  = INITIAL_SESSION_ID;
        _rebootFlag = false;
    }
    else
    {
        ++_sessionId;
    }
}

SessionEndpoint::SessionEndpoint() : SessionEntry() {}

void SessionEndpoint::clear() { SessionEntry::clear(); }

} // namespace internal

SessionManager::SessionManager(EndpointMap& map) : _endpoints(map), _multicast() {}

void SessionManager::init()
{
    _endpoints.clear();
    _multicast.clear();
}

void SessionManager::getSessionInfoForNextMulticastMessage(uint16_t& sessionId, bool& rebootFlag)
{
    sessionId  = _multicast._sessionId;
    rebootFlag = _multicast._rebootFlag;

    _multicast.increment();
}

void SessionManager::getSessionInfoForNextUnicastMessage(
    IPAddress const& ipAddress, uint16_t& sessionId, bool& rebootFlag)
{
    EndpointMap::iterator const iter = addEndpoint(ipAddress);
    if (_endpoints.end() == iter)
    {
        WARN_LOG(SOMEIP, "SessionManager: unable to add endpoint");
        sessionId  = 0U;
        rebootFlag = false;
        return;
    }

    internal::SessionEndpoint& endpoint = iter->second;

    sessionId  = endpoint._sessionId;
    rebootFlag = endpoint._rebootFlag;

    endpoint.increment();
}

// private
SessionManager::EndpointMap::iterator SessionManager::addEndpoint(IPAddress const& ipAddress)
{
    EndpointMap::iterator const iter = _endpoints.find(ipAddress);
    if (iter != _endpoints.end())
    {
        return iter; // endpoint already added
    }

    if (_endpoints.full() == true)
    {
        return _endpoints.end();
    }

    internal::SessionEndpoint& endpoint = _endpoints[ipAddress];
    endpoint.clear();

    return _endpoints.find(ipAddress);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
