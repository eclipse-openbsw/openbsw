/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RebootTracker.h"

#include "someip/logger.h"

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::ip::IPAddress;
using ::util::logger::SOMEIP;

// protected
RebootTracker::RebootTracker(EndpointMap& endpointMap) : _endpoints(endpointMap) {}

void RebootTracker::init() { _endpoints.clear(); }

bool RebootTracker::evaluate(SessionInfo const& session) const
{
    EndpointMap::iterator const iter = _endpoints.find(session.remoteIp);

    if ((iter != nullptr) && (iter != _endpoints.end()))
    {
        RebootTrackerEndpoint& endpoint = iter->second;
        if (session.isMulticast)
        {
            return endpoint.getMulticastRebootValue(session.sessionId, session.rebootFlag);
        }

        return endpoint.getUnicastRebootValue(session.sessionId, session.rebootFlag);
    }

    if (session.rebootFlag == false)
    {
        return false;
    }

    return 0 >= session.sessionId;
}

void RebootTracker::apply(SessionInfo const& session)
{
    EndpointMap::iterator const iter = addEndpoint(session.remoteIp);
    if (_endpoints.end() == iter)
    {
        WARN_LOG(SOMEIP, "RebootTracker: unable to add endpoint");
    }
    else
    {
        RebootTrackerEndpoint& endpoint = iter->second;
        bool reboot                     = false;

        if (session.isMulticast)
        {
            reboot = endpoint.getMulticastRebootValue(session.sessionId, session.rebootFlag);
            endpoint.setMulticast(session.sessionId, session.rebootFlag);
        }
        else
        {
            reboot = endpoint.getUnicastRebootValue(session.sessionId, session.rebootFlag);
            endpoint.setUnicast(session.sessionId, session.rebootFlag);
        }

        if (reboot)
        {
            (void)_endpoints.erase(session.remoteIp);
        }
    }
}

RebootTracker::EndpointMap::iterator RebootTracker::addEndpoint(IPAddress const& remoteIp)
{
    EndpointMap::iterator const iter = _endpoints.find(remoteIp);
    if (iter != _endpoints.end())
    {
        return iter; // endpoint already added
    }

    if (_endpoints.full() == true)
    {
        return _endpoints.end();
    }

    RebootTrackerEndpoint& endpoint = _endpoints[remoteIp];
    endpoint.initActiveReboot();

    return _endpoints.find(remoteIp);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
