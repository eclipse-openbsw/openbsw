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

#include "someip/RebootTrackerEndpoint.h"

#include <ip/IPAddress.h>

#include <etl/flat_map.h>
#include <cstdint>

namespace someip
{
/**
 * Information about last SD message
 */
struct SessionInfo
{
    SessionInfo(
        ::ip::IPAddress const& remoteIp,
        bool const isMulticast,
        uint16_t const sessionId,
        bool const rebootFlag)
    : remoteIp(remoteIp), isMulticast(isMulticast), sessionId(sessionId), rebootFlag(rebootFlag)
    {}

    ::ip::IPAddress const& remoteIp;
    bool isMulticast;
    uint16_t sessionId;
    bool rebootFlag;
};

/**
 * Responsible to detect reboot of remote systems.
 */
class RebootTracker
{
public:
    /**
     * Lifecycle method that clears EndpointMap of RebootTracker.
     */
    void init();

    /**
     * Checks whether a reboot has occured based on information about
     * the latest SD message.
     *
     * \param sessionInfo information about the last SD message
     *
     * \return true if a reboot was detected.
     */
    bool evaluate(SessionInfo const& session) const;

    /**
     * Applies changes based on information about the latest
     * SD message.
     *
     * \param sessionInfo information about the last SD message
     *
     */
    void apply(SessionInfo const& session);

protected:
    using EndpointMap
        = ::etl::iflat_map<::ip::IPAddress, RebootTrackerEndpoint, ::ip::IPAddressCompareLess>;

    explicit RebootTracker(EndpointMap& endpointMap);

private:
    EndpointMap::iterator addEndpoint(::ip::IPAddress const& remoteIp);

    EndpointMap& _endpoints;
};

namespace declare
{
template<uint16_t NUM_ENDPOINTS>
class RebootTracker : public ::someip::RebootTracker
{
public:
    RebootTracker() : ::someip::RebootTracker(_endpointMap) {}

private:
    ::etl::
        flat_map<::ip::IPAddress, RebootTrackerEndpoint, NUM_ENDPOINTS, ::ip::IPAddressCompareLess>
            _endpointMap;
};
} // namespace declare
} // namespace someip
