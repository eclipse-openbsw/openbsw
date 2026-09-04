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

#include "someip/ISdMessageParser.h"
#include "someip/IServiceAnnouncer.h"
#include "someip/IServiceRegistry.h"
#include "someip/RebootTracker.h"
#include "someip/SdOptions.h"
#include "someip/SomeIpMessage.h"

#include <ip/IPAddress.h>

namespace someip
{
class SdMessageParser : public ISdMessageParser
{
public:
    SdMessageParser(
        IServiceRegistry& serviceRegistry,
        IServiceAnnouncer& serviceAnnouncer,
        RebootTracker& rebootTracker,
        uint8_t subnetId,
        ::ip::IPAddress const& localIp,
        AdditionalSDCheck additionalSDCheck);

    /**
     * Lifecycle function that initializes RebootTracker.
     */
    void init();

    /** \see ISdMessageParser
     * Function that triggers further parsing of a passed message if
     * the message is superficially valid and updates RebootTracker in case
     * of a detected reboot and/or in case of successfully triggering further
     * parsing. To ensure superficial validness protocol version, interface
     * version and client ID are checked.
     */
    void handleMessage(
        SomeIpMessage const& message,
        ::ip::IPEndpoint const& sourceEndpoint,
        bool isMulticast) override;

private:
    bool parseMessage(
        ::etl::span<uint8_t const> const& payload,
        ::ip::IPEndpoint const& sourceEndpoint,
        bool receivedByMulticast);

    bool parseEntry(
        ::etl::span<uint8_t const> const& entry,
        ::ip::IPEndpoint const& sourceEndpoint,
        bool receivedByMulticast,
        bool sdFlagUnicast,
        SdOptions& options);

    void handleEntryFind(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        ttl::type ttl,
        minor_version::type minorVersion,
        ::ip::IPAddress const& sourceAddress,
        bool sdFlagUnicast);

    void handleEntryOffer(
        SdOptions const& options,
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        ttl::type ttl,
        minor_version::type minorVersion,
        ::ip::IPEndpoint const& sourceEndpoint);

    void handleEntrySubscribe(
        SdOptions const& options,
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        eventgroup_id::type eventgroup,
        ttl::type ttl,
        uint16_t reserved,
        ::ip::IPEndpoint const& sourceEndpoint);

    void handleEntrySubscribeAck(
        SdOptions const& options,
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        eventgroup_id::type eventgroup,
        ttl::type ttl,
        ::ip::IPAddress const& sourceAddress);

    IServiceRegistry& _serviceRegistry;
    IServiceAnnouncer& _serviceAnnouncer;

    uint8_t _subnetId;
    ::ip::IPAddress const& _localIp;

    RebootTracker& _rebootTracker;

    // function pointer that can be used to inject additional SD check
    AdditionalSDCheck _additionalSDCheck;
};

} // namespace someip
