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

#include "someip/IServiceAnnouncer.h"
#include "someip/IServiceRegistry.h"
#include "someip/QueryManager.h"
#include "someip/SdMessageBuilder.h"
#include "someip/ServiceAnnouncerTask.h"
#include "someip/ServiceManager.h"
#include "someip/SessionManager.h"
#include "someip/SomeIpConstants.h"

#include <async/Types.h>
#include <async/util/Call.h>

#include <ip/IPAddress.h>

#include <etl/array.h>
#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>
#include <etl/pool.h>

namespace common
{
class ITimeoutManager2;
}

namespace someip
{
class ServiceAnnouncer : public IServiceAnnouncer
{
public:
    ServiceAnnouncer(
        INetwork& network,
        ServiceManager& serviceManager,
        IServiceRegistry& serviceRegistry,
        ::async::ContextType const ethernetContext,
        QueryManager& queryManager,
        SessionManager& sessionManager);

    void init();
    void shutdown();

    void start() override;
    void stop() override;

    void respondToFindService(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        minor_version::type minorVersion,
        ttl::type ttl,
        ::ip::IPAddress const& sourceIpAddress,
        bool unicast) override;

    void respondToSubscribe(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        uint16_t reserved,
        eventgroup_id::type eventgroup,
        ttl::type ttl,
        ::ip::IPAddress const& sourceIpAddress,
        ::ip::IPAddress const& endpointIpAddress,
        uint16_t endpointPort,
        uint8_t endpointProto) override;

    void sendSubscribeAck(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup,
        major_version::type majorVersion,
        uint16_t reserved,
        ttl::type ttl,
        ::ip::IPAddress const& sourceIpAddress) override;

    void sendSubscribeNack(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup,
        major_version::type majorVersion,
        uint16_t reserved,
        ::ip::IPAddress const& sourceIpAddress) override;

    void sendSubscribeAckMulticast(
        service_id::type serviceId,
        instance_id::type instanceId,
        eventgroup_id::type eventgroup,
        major_version::type majorVersion,
        uint16_t reserved,
        ttl::type ttl,
        ::ip::IPAddress const& endpointAddress,
        uint16_t endpointPort,
        ::ip::IPAddress const& sourceIpAddress) override;

    void
    subscribe(ServiceDescription const& service, ::ip::IPAddress const& sourceAddress) override;
    void
    unsubscribe(ServiceDescription const& service, ::ip::IPAddress const& sourceAddress) override;
    void find(ServiceDescription const& service) override;
    void offer(ServiceDescription const& service) override;
    void stopOffer(ServiceDescription const& service) override;

    void sendStopOffers() override;

    void cyclic();

    size_t getNumPendingBrowseRequests() const { return _pendingBrowseRequests.size(); }

    size_t getNumPendingTxMessages() const { return _pendingTxMessages.size(); }

protected:
    void checkPendingTasks(uint64_t now);
    void addProvidedServices(uint64_t now);
    void addQueries() const;

private:
    static uint32_t const REQ_RES_MIN_DELAY_MS = 0U;
    static uint32_t const REQ_RES_MAX_DELAY_MS = 0U;
    static uint32_t const CYCLE_TIME_MS        = 15U;
    static uint8_t const MAX_NUM_QUEUED_TASKS  = 32U;

    static uint8_t const MAX_NUM_ENTRIES_PER_MESSAGE
        = 86U; // max. possible number of entries in payload ((1400 - 12) / 16 = 86)
    static uint8_t const MAX_NUM_OPTIONS_PER_MESSAGE = 16U;

    void sendDueMessages();
    void enqueueTxMessage(ServiceAnnouncerTask const& txMessage);
    void checkPendingTasksAndSendDueMessages();
    void triggerEventTimeout();
    void executeTask(ServiceAnnouncerTask const& task);
    void initializeMulticastMessage();
    void initializeUnicastMessage(::ip::IPAddress const& destinationAddress);
    void finalizeMessage();
    void getSessionInfoForNextMessage(uint16_t& sessionId, bool& rebootFlag);
    void processTaskOffer(ServiceAnnouncerTask const& task);
    void processTaskBrowseResults(ServiceAnnouncerTask const& task) const;
    void processTaskSubscribe(ServiceAnnouncerTask const& task);
    void processTaskSubscribeAck(ServiceAnnouncerTask const& task);
    void processTaskSubscribeAckMulticast(ServiceAnnouncerTask const& task);
    void processTaskSubscribeNack(ServiceAnnouncerTask const& task);
    void processTaskUnsubscribe(ServiceAnnouncerTask const& task);
    void releaseTask(ServiceAnnouncerTask& task);
    void addOffer(ServiceDescription& service);
    void addStopOffer(ServiceDescription& service);
    void addSubscribe(ServiceDescription& service);
    void addUnsubscribe(ServiceDescription& service);
    void addSubscribeAck(ServiceDescription const& service);
    void addSubscribeAckMulticast(ServiceDescription const& service);
    void addSubscribeNack(ServiceDescription const& service);
    void resetIfFull(SdMessageReturnCode);
    void sendMessage();

    INetwork& _network;
    ServiceManager& _serviceManager;
    IServiceRegistry& _serviceRegistry;

    ::async::ContextType const _ethernetContext;
    ::async::Function _cyclicFunction;
    ::async::TimeoutType _cyclicTimeout;
    ::async::Function _eventFunction;
    ::async::TimeoutType _eventTimeout;

    SdMessageBuilder _messageBuilder;

    ::etl::array<uint8_t, SD_PACKET_MAX_SIZE> _messageBuffer;

    bool _isStarted;
    ::ip::IPAddress _destinationAddress{};

    using tTaskPool = ::etl::pool<ServiceAnnouncerTask, MAX_NUM_QUEUED_TASKS>;
    using tPendingTaskList
        = ::etl::intrusive_forward_list<ServiceAnnouncerTask, ::etl::forward_link<0>>;
    tTaskPool _taskPool;
    tPendingTaskList _pendingBrowseRequests;
    tPendingTaskList _pendingTxMessages;

    QueryManager& _queryManager;
    SessionManager& _sessionManager;
};

} // namespace someip
