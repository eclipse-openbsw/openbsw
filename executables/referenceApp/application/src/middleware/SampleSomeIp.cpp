/********************************************************************************
 * Copyright (c) 2026 KPIT Group
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/SampleSomeIp.h"

#include <ets/EtsCodecs.h>
#include <ets/EtsCommon.h>
#include <middleware/ClusterId.h>
#include <middleware/core/ClusterConnection.h>
#include <someip/SdConfig.h>
#include <someip/ServiceDescription.h>
#include <someip/SomeIpConstants.h>

#include <etl/span.h>
#include <etl/utility.h>

#include <new>

#ifdef SOMEIP_SAMPLE_ZONAL_PLATFORM1
#include <middleware/ClusterCluster3.h>
#else
#include <middleware/ClusterCluster2.h>
#endif

namespace middlewaresample
{

namespace
{

namespace Ids = ::ets::Ets::internal;
using Kind    = ::middleware::someip::MemberBinding::Kind;
using ::middleware::core::ClusterId;

// The application this node runs in the FLYNC model, and its cluster.
#ifdef SOMEIP_SAMPLE_ZONAL_PLATFORM1
// zonal_platform1: application2 provides instance 2 and consumes instance 1.
constexpr ClusterId OWN_CLUSTER             = ClusterId::Cluster3;
constexpr ClusterId PEER_CLUSTER            = ClusterId::Cluster2;
constexpr Ids::InstanceId PROVIDED_INSTANCE = Ids::InstanceId_2;
constexpr Ids::InstanceId CONSUMED_INSTANCE = Ids::InstanceId_1;

::middleware::core::ClusterConnection& ownConnection()
{
    return ::middleware::ClusterConnectionCluster3ToCluster2;
}
#else
// high_performance_compute: application1 provides instance 1 and consumes instance 2.
constexpr ClusterId OWN_CLUSTER             = ClusterId::Cluster2;
constexpr ClusterId PEER_CLUSTER            = ClusterId::Cluster3;
constexpr Ids::InstanceId PROVIDED_INSTANCE = Ids::InstanceId_1;
constexpr Ids::InstanceId CONSUMED_INSTANCE = Ids::InstanceId_2;

::middleware::core::ClusterConnection& ownConnection()
{
    return ::middleware::ClusterConnectionCluster2ToCluster3;
}
#endif

// From the FLYNC model: the service's version, its eventgroups, and the SD
// configuration and multicast group of its deployments.
constexpr uint8_t MAJOR_VERSION         = 1U;
constexpr uint32_t MINOR_VERSION        = 1U;
constexpr uint16_t EVENTGROUP_ALL       = 0x0002U;
constexpr uint16_t EVENTGROUP_MULTICAST = 0x0006U;
constexpr uint32_t TTL_S                = 10U;

constexpr uint32_t SD_INITIAL_DELAY_MS          = 50U;
constexpr uint32_t SD_REPETITIONS_BASE_DELAY_MS = 300U;
constexpr uint32_t SD_REPETITIONS_MAX           = 3U;
constexpr uint32_t SD_CYCLIC_OFFER_DELAY_MS     = 1000U;

// The group's port is the service port: cpp2someip delivers a consumed event only
// if it arrives on the port the subscription was made from.
constexpr auto EVENT_MULTICAST_IP       = ::ip::make_ip4(239U, 2U, 41U, 16U);
constexpr uint16_t EVENT_MULTICAST_PORT = SampleSomeIp::PORT;

constexpr uint16_t FIELD_EVENTGROUPS[] = {EVENTGROUP_ALL};
constexpr uint16_t EVENT_EVENTGROUPS[] = {EVENTGROUP_ALL, EVENTGROUP_MULTICAST};

::middleware::someip::SomeIpBridgeConfig makeConfig()
{
    ::middleware::someip::SomeIpBridgeConfig config{};
    config.majorVersion          = MAJOR_VERSION;
    config.minorVersion          = MINOR_VERSION;
    config.instanceId            = PROVIDED_INSTANCE;
    config.providerPort          = SampleSomeIp::PORT;
    config.consumedInstanceId    = CONSUMED_INSTANCE;
    config.consumedEventGroupId  = EVENTGROUP_ALL;
    config.proto                 = ::someip::proto::SD_L4_PROTO_UDP;
    config.ttl                   = TTL_S;
    config.eventMulticastAddress = EVENT_MULTICAST_IP;
    config.eventMulticastPort    = EVENT_MULTICAST_PORT;
    config.sdConfig              = ::someip::SdOfferConfig(
        SD_INITIAL_DELAY_MS,
        SD_REPETITIONS_BASE_DELAY_MS,
        SD_REPETITIONS_MAX,
        SD_CYCLIC_OFFER_DELAY_MS);
    config.findConfig
        = ::someip::SdConfig(SD_INITIAL_DELAY_MS, SD_REPETITIONS_BASE_DELAY_MS, SD_REPETITIONS_MAX);
    return config;
}

} // namespace

SampleSomeIp::SampleSomeIp(
    ::someip::ISomeIpStack& stack,
    ::someip::IEventSender& eventSender,
    ::someip::INetwork& network,
    ::someip::IRpcSender& rpcSender)
: _stack(stack)
, _endpoint(stack, eventSender, network, rpcSender, Ids::SERVICE_ID, makeConfig())
, _connection(::etl::to_underlying(OWN_CLUSTER), ::etl::to_underlying(PEER_CLUSTER))
{}

void SampleSomeIp::init()
{
    // The endpoint reaches the middleware through the connection, so it is added
    // before anything can arrive for it. offerService() publishes what is bound.
    _connection.add(_endpoint);
    bindMembers();
    _endpoint.offerService();
    offerEventGroup(_eventGroupAll, EVENTGROUP_ALL);
    offerEventGroup(_eventGroupMulticast, EVENTGROUP_MULTICAST);

    // The skeleton and the proxy register with whichever configuration the
    // connection holds when they are initialized, so the redirect comes first.
    // The generated connection is referenced by the constexpr instances database
    // and has no setter: it is re-constructed in place.
    ownConnection().~ClusterConnection();
    ::new (&ownConnection())::middleware::core::ClusterConnection(_connection);
}

void SampleSomeIp::run(::async::ContextType const context)
{
    ::async::scheduleAtFixedRate(
        context, *this, _timeout, CYCLE_MS, ::async::TimeUnit::MILLISECONDS);
}

void SampleSomeIp::shutdown()
{
    _timeout.cancel();
    if (_isInitialized)
    {
        _consumer.deInit();
        _provider.deInit();
    }
}

void SampleSomeIp::execute()
{
    if (!_isInitialized)
    {
        // A skeleton must be initialized on the task it sends from: this one, the
        // stack's. SomeIpSystem's init() runs on another task.
        static_cast<void>(_provider.init(PROVIDED_INSTANCE));
        _consumer.init(CONSUMED_INSTANCE, OWN_CLUSTER);
        _isInitialized = true;
        ::util::logger::Logger::info(
            ::util::logger::DEMO,
            "[MiddlewareSample] providing instance %u, consuming instance %u, on UDP port %u",
            static_cast<unsigned>(PROVIDED_INSTANCE),
            static_cast<unsigned>(CONSUMED_INSTANCE),
            static_cast<unsigned>(PORT));
        return;
    }
    _provider.publish();
    // Until SD finds the peer's instance, a request to it would only fail.
    if (_endpoint.isConsumedInstanceAvailable())
    {
        _consumer.step();
    }
}

void SampleSomeIp::bindMembers()
{
    ::etl::span<uint16_t const> const fieldEventGroups{FIELD_EVENTGROUPS};
    ::etl::span<uint16_t const> const eventEventGroups{EVENT_EVENTGROUPS};

    // This node's instance.
    _endpoint.bindMethod(Ids::echoUint8_id, Kind::RequestResponse, &_echoRequest, &_echoResponse);
    _endpoint.bindEvent(Ids::event_TestFieldUINT8_id, _fieldNotification, fieldEventGroups);
    _endpoint.bindMethod(
        Ids::get_TestFieldUINT8_id, Kind::ResponseOnly, nullptr, &_fieldGetResponse);
    _endpoint.bindMethod(
        Ids::set_TestFieldUINT8_id, Kind::RequestResponse, &_fieldSetRequest, &_fieldSetResponse);
    _endpoint.bindEvent(
        Ids::event_TestEventUINT8Multicast_id, _event, eventEventGroups, /*isMulticast=*/true);

    // The peer's instance.
    _endpoint.bindConsumedMethod(
        Ids::echoUint8_id, Kind::RequestResponse, &_peerEchoRequest, &_peerEchoResponse);
    _endpoint.bindConsumedEvent(Ids::event_TestFieldUINT8_id, _peerFieldNotification);
    _endpoint.bindConsumedMethod(
        Ids::get_TestFieldUINT8_id, Kind::ResponseOnly, nullptr, &_peerFieldGetResponse);
    _endpoint.bindConsumedMethod(
        Ids::set_TestFieldUINT8_id,
        Kind::RequestResponse,
        &_peerFieldSetRequest,
        &_peerFieldSetResponse);
    _endpoint.bindConsumedEvent(Ids::event_TestEventUINT8Multicast_id, _peerEvent);
}

void SampleSomeIp::offerEventGroup(::someip::ProvidedService& service, uint16_t const eventGroupId)
{
    ::someip::ServiceDescription desc{};
    desc.minorVersion = MINOR_VERSION;
    desc.ttl          = TTL_S;
    desc.serviceId    = Ids::SERVICE_ID;
    desc.instanceId   = PROVIDED_INSTANCE;
    desc.eventGroup   = eventGroupId;
    // Both eventgroups carry the multicast event, so both offer the multicast
    // group: that is what puts it in the Ack and tells a subscriber where to listen.
    desc.ipAddress    = EVENT_MULTICAST_IP;
    desc.port         = EVENT_MULTICAST_PORT;
    desc.proto        = ::someip::proto::SD_L4_PROTO_UDP;
    desc.majorVersion = MAJOR_VERSION;
    service.setHandler(_endpoint);
    service.description = desc;
    service.setSdConfig(::someip::SdOfferConfig(
        SD_INITIAL_DELAY_MS,
        SD_REPETITIONS_BASE_DELAY_MS,
        SD_REPETITIONS_MAX,
        SD_CYCLIC_OFFER_DELAY_MS));
    static_cast<void>(_stack.registerProvidedService(service));
}

} // namespace middlewaresample
