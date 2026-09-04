/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceAnnouncer.h"

#include "bsp/timer/SystemTimer.h"
#include "someip/INetwork.h"
#include "someip/IServiceRegistry.h"
#include "someip/NetworkChannel.h"
#include "someip/ProvidedService.h"
#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/Statistics.h"
#include "someip/logger.h"

#include <etl/algorithm.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

ServiceAnnouncer::ServiceAnnouncer(
    INetwork& network,
    ServiceManager& serviceManager,
    IServiceRegistry& serviceRegistry,
    ::async::ContextType const ethernetContext,
    QueryManager& queryManager,
    SessionManager& sessionManager)
: _network(network)
, _serviceManager(serviceManager)
, _serviceRegistry(serviceRegistry)
, _ethernetContext(ethernetContext)
, _cyclicFunction(
      ::async::Function::CallType::create<ServiceAnnouncer, &ServiceAnnouncer::cyclic>(*this))
, _cyclicTimeout()
, _eventFunction(
      ::async::Function::CallType::
          create<ServiceAnnouncer, &ServiceAnnouncer::checkPendingTasksAndSendDueMessages>(*this))
, _eventTimeout()
, _messageBuilder()
, _messageBuffer()
, _isStarted(false)
, _taskPool()
, _pendingBrowseRequests()
, _pendingTxMessages()
, _queryManager(queryManager)
, _sessionManager(sessionManager)
{}

void ServiceAnnouncer::init()
{
    _sessionManager.init();
    etl::fill(_messageBuffer.begin(), _messageBuffer.end(), 0);

    _taskPool.release_all();
    _pendingBrowseRequests.clear();
    _pendingTxMessages.clear();
}

void ServiceAnnouncer::shutdown() { stop(); }

void ServiceAnnouncer::start()
{
    if (!_isStarted)
    {
        async::scheduleAtFixedRate(
            _ethernetContext,
            _cyclicFunction,
            _cyclicTimeout,
            CYCLE_TIME_MS,
            ::async::TimeUnit::MILLISECONDS);
        _isStarted = true;
    }
}

void ServiceAnnouncer::stop()
{
    if (_isStarted)
    {
        checkPendingTasks(getSystemTimeMs32Bit());
        sendStopOffers();
        _cyclicTimeout.cancel();
        _eventTimeout.cancel();
        _isStarted = false;
    }
}

void ServiceAnnouncer::respondToFindService(
    service_id::type serviceId,
    instance_id::type instanceId,
    major_version::type majorVersion,
    minor_version::type minorVersion,
    ttl::type,
    ::ip::IPAddress const& sourceIpAddress,
    bool unicast)
{
    if ((instanceId == instance_id::ANY) || (majorVersion == major_version::ANY)
        || (minorVersion == minor_version::ANY))
    {
        if (!_taskPool.full())
        {
            ServiceAnnouncerTask& task = *_taskPool.create();
            task.init(
                serviceId,
                instance_id::ANY,
                eventgroup_id::ALL,
                ttl::INVALID,
                majorVersion,
                minorVersion);

            uint32_t const delay = REQ_RES_MAX_DELAY_MS; // TODO: determine delay randomly
            task.setDestinationAddress(sourceIpAddress);
            task.setUnicast(unicast);
            task.setTimestamp(getSystemTimeMs32Bit() + delay);
            _pendingBrowseRequests.push_front(task);

            if (_taskPool.full())
            {
                checkPendingTasks(getSystemTimeMs32Bit());
            }
        }
        else
        {
            WARN_LOG(SOMEIP, "ServiceAnnouncer::respondToFindService() task pool is full");
        }
    }
    else
    {
        auto service         = ::someip::make<ServiceDescription>();
        service.serviceId    = serviceId;
        service.majorVersion = majorVersion;
        service.minorVersion = minorVersion; // don't care
        service.instanceId   = instanceId;

        ProvidedService const* const providedService = _serviceManager.getService(service);

        // only answer if service is in its MAIN phase
        if ((providedService != nullptr)
            && (ProvidedService::ProvidedServiceState::MAIN_PHASE == providedService->getState()))
        {
            if (!_taskPool.full())
            {
                ServiceAnnouncerTask& task = *_taskPool.create();

                task.initFrom(
                    providedService->description,
                    sourceIpAddress,
                    unicast,
                    getSystemTimeMs32Bit() + REQ_RES_MIN_DELAY_MS,
                    ServiceAnnouncerTask::TaskType::TASK_ANNOUNCE);

                _pendingTxMessages.push_front(task);

                if (_taskPool.full())
                {
                    checkPendingTasks(getSystemTimeMs32Bit());
                }
                else
                {
                    triggerEventTimeout();
                }
            }
            else
            {
                WARN_LOG(SOMEIP, "ServiceAnnouncer::respondToFindService() task pool is full");
            }
        }
    }
}

void ServiceAnnouncer::respondToSubscribe(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    uint16_t const reserved,
    eventgroup_id::type const eventgroup,
    ttl::type const ttl,
    ::ip::IPAddress const& sourceIpAddress,
    ::ip::IPAddress const& endpointIpAddress,
    uint16_t const endpointPort,
    uint8_t const endpointProto)
{
    IServiceRegistry::SubscriptionResult result;
    result = _serviceRegistry.subscribeReceived(
        serviceId,
        instanceId,
        majorVersion,
        eventgroup,
        ttl,
        endpointIpAddress,
        endpointPort,
        endpointProto);

    if (IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK == result)
    {
        sendSubscribeAck(
            serviceId, instanceId, eventgroup, majorVersion, reserved, ttl, sourceIpAddress);
    }
    else if (IServiceRegistry::SubscriptionResult::SUBSCRIBE_OK_MULTICAST == result)
    {
        auto service         = ::someip::make<ServiceDescription>();
        service.serviceId    = serviceId;
        service.instanceId   = instanceId;
        service.majorVersion = majorVersion;
        service.eventGroup   = eventgroup;

        ProvidedService const* const providedService = _serviceManager.getEventGroup(service);

        if (providedService != nullptr)
        {
            sendSubscribeAckMulticast(
                serviceId,
                instanceId,
                eventgroup,
                majorVersion,
                reserved,
                ttl,
                providedService->description.ipAddress,
                providedService->description.port,
                sourceIpAddress);
        }
    }
    else if (IServiceRegistry::SubscriptionResult::SUBSCRIBE_ERROR == result)
    {
        sendSubscribeNack(
            serviceId, instanceId, eventgroup, majorVersion, reserved, sourceIpAddress);
    }
    else
    {
        // misra
    }
}

void ServiceAnnouncer::sendSubscribeAck(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroup,
    major_version::type const majorVersion,
    uint16_t const reserved,
    ttl::type const ttl,
    ::ip::IPAddress const& sourceIpAddress)
{
    ServiceAnnouncerTask task;
    task.init(
        serviceId, instanceId, eventgroup, ttl, majorVersion, static_cast<uint32_t>(reserved));
    task.setDestinationAddress(sourceIpAddress);
    task.setTimestamp(getSystemTimeMs32Bit() + REQ_RES_MIN_DELAY_MS);
    task.setType(ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE_ACK);
    enqueueTxMessage(task);
}

void ServiceAnnouncer::sendSubscribeNack(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroup,
    major_version::type const majorVersion,
    uint16_t const reserved,
    ::ip::IPAddress const& sourceIpAddress)
{
    ServiceAnnouncerTask task;
    task.init(serviceId, instanceId, eventgroup, 0U, majorVersion, static_cast<uint32_t>(reserved));
    task.setDestinationAddress(sourceIpAddress);
    task.setTimestamp(getSystemTimeMs32Bit() + REQ_RES_MIN_DELAY_MS);
    task.setType(ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE_NACK);
    enqueueTxMessage(task);
}

void ServiceAnnouncer::sendSubscribeAckMulticast(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    eventgroup_id::type const eventgroup,
    major_version::type const majorVersion,
    uint16_t const reserved,
    ttl::type const ttl,
    ::ip::IPAddress const& endpointAddress,
    uint16_t const endpointPort,
    ::ip::IPAddress const& sourceIpAddress)
{
    ServiceAnnouncerTask task;
    task.init(
        serviceId, instanceId, eventgroup, ttl, majorVersion, static_cast<uint32_t>(reserved));
    task.setProto(proto::SD_L4_PROTO_UDP);
    task.setEndpoint(endpointAddress, endpointPort);

    task.setDestinationAddress(sourceIpAddress);
    task.setTimestamp(getSystemTimeMs32Bit() + REQ_RES_MIN_DELAY_MS);
    task.setType(ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE_ACK_MULTICAST);
    enqueueTxMessage(task);
}

void ServiceAnnouncer::subscribe(
    ServiceDescription const& service, ::ip::IPAddress const& sourceAddress)
{
    if (!containsEventGroup(service))
    {
        return; // only event groups can be subscribed
    }

    ServiceAnnouncerTask task;

    task.initFrom(
        service,
        sourceAddress,
        false,
        getSystemTimeMs32Bit() + REQ_RES_MIN_DELAY_MS,
        ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE);

    task.setMinorVersion(minor_version::INVALID);
    task.setEndpoint(_network.getLocalIp(), service.port);
    enqueueTxMessage(task);
}

void ServiceAnnouncer::unsubscribe(
    ServiceDescription const& service, ::ip::IPAddress const& sourceAddress)
{
    if (!containsEventGroup(service))
    {
        return; // only event groups can be unsubscribed
    }

    ServiceAnnouncerTask task;

    task.initFrom(
        service,
        sourceAddress,
        false,
        getSystemTimeMs32Bit() + REQ_RES_MIN_DELAY_MS,
        ServiceAnnouncerTask::TaskType::TASK_UNSUBSCRIBE);

    task.setMinorVersion(minor_version::INVALID);
    task.setEndpoint(_network.getLocalIp(), service.port);
    enqueueTxMessage(task);
}

void ServiceAnnouncer::sendStopOffers()
{
    initializeMulticastMessage();
    _serviceManager.triggerStopOffers(); // callback stopOffer
    finalizeMessage();
}

void ServiceAnnouncer::cyclic() { checkPendingTasksAndSendDueMessages(); }

void ServiceAnnouncer::sendDueMessages()
{
    initializeMulticastMessage();
    addProvidedServices(getSystemTimeMs32Bit());
    addQueries();
    finalizeMessage();
}

void ServiceAnnouncer::enqueueTxMessage(ServiceAnnouncerTask const& txMessage)
{
    if (!_taskPool.full())
    {
        ServiceAnnouncerTask& task = *_taskPool.create();
        task                       = txMessage;
        _pendingTxMessages.push_front(task);

        if (_taskPool.full())
        {
            checkPendingTasks(getSystemTimeMs32Bit());
        }
        else
        {
            triggerEventTimeout();
        }
    }
    else
    {
        WARN_LOG(SOMEIP, "ServiceAnnouncer::enqueueTxMessage() task pool is empty");
    }
}

void ServiceAnnouncer::checkPendingTasks(uint64_t const now)
{
    tPendingTaskList::iterator itr  = _pendingBrowseRequests.begin();
    tPendingTaskList::iterator prev = _pendingBrowseRequests.before_begin();
    while (itr != _pendingBrowseRequests.end())
    {
        if (itr->getTimestamp() <= now)
        {
            initializeUnicastMessage(itr->getDestinationAddress());
            ::ip::IPAddress const currentDestination = itr->getDestinationAddress();
            processTaskBrowseResults(*itr);
            releaseTask(*itr);
            itr = _pendingBrowseRequests.erase_after(prev);

            tPendingTaskList::iterator followUpItr     = itr;
            tPendingTaskList::iterator prevFollowUpItr = prev;
            while ((followUpItr != _pendingBrowseRequests.end())
                   && (followUpItr->getDestinationAddress() == currentDestination))
            {
                if (followUpItr->getTimestamp() <= now)
                {
                    processTaskBrowseResults(*followUpItr);
                    releaseTask(*followUpItr);
                    followUpItr = _pendingBrowseRequests.erase_after(prevFollowUpItr);
                }
                else
                {
                    prevFollowUpItr = followUpItr;
                    ++followUpItr;
                }
                prev = prevFollowUpItr;
                itr  = followUpItr;
            }
            finalizeMessage();
        }
        else
        {
            prev = itr;
            ++itr;
        }
    }

    while (!_pendingTxMessages.empty())
    {
        ServiceAnnouncerTask& task = _pendingTxMessages.front();
        initializeUnicastMessage(task.getDestinationAddress());
        ::ip::IPAddress const currentDestination = task.getDestinationAddress();
        executeTask(task);
        _pendingTxMessages.pop_front();
        releaseTask(task);

        while ((!_pendingTxMessages.empty())
               && (_pendingTxMessages.front().getDestinationAddress() == currentDestination))
        {
            ServiceAnnouncerTask& followUpTask = _pendingTxMessages.front();
            executeTask(followUpTask);
            _pendingTxMessages.pop_front();
            releaseTask(followUpTask);
        }
        finalizeMessage();
    }
}

void ServiceAnnouncer::checkPendingTasksAndSendDueMessages()
{
    checkPendingTasks(getSystemTimeMs32Bit());
    sendDueMessages();
}

void ServiceAnnouncer::triggerEventTimeout()
{
    if (!_isStarted)
    {
        return;
    }

    _eventTimeout.cancel();
    async::schedule(
        _ethernetContext, _eventFunction, _eventTimeout, 1U, ::async::TimeUnit::MILLISECONDS);
}

void ServiceAnnouncer::executeTask(ServiceAnnouncerTask const& task)
{
    switch (task.getType())
    {
        case ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE:
        {
            processTaskSubscribe(task);
            break;
        }
        case ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE_ACK:
        {
            processTaskSubscribeAck(task);
            break;
        }
        case ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE_NACK:
        {
            processTaskSubscribeNack(task);
            break;
        }
        case ServiceAnnouncerTask::TaskType::TASK_SUBSCRIBE_ACK_MULTICAST:
        {
            processTaskSubscribeAckMulticast(task);
            break;
        }
        case ServiceAnnouncerTask::TaskType::TASK_UNSUBSCRIBE:
        {
            processTaskUnsubscribe(task);
            break;
        }
        case ServiceAnnouncerTask::TaskType::TASK_ANNOUNCE:
        {
            processTaskOffer(task);
            break;
        }
        default:
        {
            WARN_LOG(SOMEIP, "ServiceAnnouncer::executeTask() invalid task %d", task.getType());
            break;
        }
    }
}

void ServiceAnnouncer::initializeMulticastMessage()
{
    /*
     * No need to check the result of 'startMessage' if we know that provided message buffer
     * is enough to contain at least header at ServiceAnnouncer construction stage.
     * Message buffer is declared as a static array.
     */
    (void)_messageBuilder.startMessage(_messageBuffer);
    _destinationAddress = _network.getMulticastIp();
}

void ServiceAnnouncer::initializeUnicastMessage(::ip::IPAddress const& destinationAddress)
{
    /*
     * No need to check the result of 'startMessage' if we know that provided message buffer
     * is enough to contain at least header at ServiceAnnouncer construction stage.
     * Message buffer is declared as a static array.
     */
    (void)_messageBuilder.startMessage(_messageBuffer);
    _destinationAddress = destinationAddress;
}

void ServiceAnnouncer::finalizeMessage()
{
    if (_messageBuilder.isEmpty())
    {
        _messageBuilder.discardMessage();
    }
    else
    {
        uint16_t sessionId = 0U;
        bool rebootFlag    = false;
        getSessionInfoForNextMessage(sessionId, rebootFlag);
        (void)_messageBuilder.finishMessage(sessionId, rebootFlag);

        sendMessage();
    }
    etl::fill(_messageBuffer.begin(), _messageBuffer.end(), 0);
}

void ServiceAnnouncer::getSessionInfoForNextMessage(uint16_t& sessionId, bool& rebootFlag)
{
    if (_destinationAddress == _network.getMulticastIp())
    {
        _sessionManager.getSessionInfoForNextMulticastMessage(sessionId, rebootFlag);
    }
    else
    {
        _sessionManager.getSessionInfoForNextUnicastMessage(
            _destinationAddress, sessionId, rebootFlag);
    }
}

void ServiceAnnouncer::processTaskOffer(ServiceAnnouncerTask const& task)
{
    auto service = ::someip::make<ServiceDescription>();
    task.copyTo(service);

    addOffer(service);
}

void ServiceAnnouncer::processTaskBrowseResults(ServiceAnnouncerTask const& task) const
{
    _serviceManager.triggerOffers(task); // callback offer
}

void ServiceAnnouncer::processTaskSubscribe(ServiceAnnouncerTask const& task)
{
    if (!task.containsEventGroup())
    {
        return;
    }

    auto service = ::someip::make<ServiceDescription>();
    task.copyTo(service);

    addSubscribe(service);
}

void ServiceAnnouncer::processTaskSubscribeAck(ServiceAnnouncerTask const& task)
{
    auto service = ::someip::make<ServiceDescription>();
    task.copyTo(service);

    addSubscribeAck(service);
}

void ServiceAnnouncer::processTaskSubscribeNack(ServiceAnnouncerTask const& task)
{
    auto service = ::someip::make<ServiceDescription>();
    task.copyTo(service);

    addSubscribeNack(service);
}

void ServiceAnnouncer::processTaskSubscribeAckMulticast(ServiceAnnouncerTask const& task)
{
    if (!task.containsEventGroup())
    {
        return;
    }

    auto service = ::someip::make<ServiceDescription>();
    task.copyTo(service);

    addSubscribeAckMulticast(service);
}

void ServiceAnnouncer::processTaskUnsubscribe(ServiceAnnouncerTask const& task)
{
    if (!task.containsEventGroup())
    {
        return;
    }

    auto service = ::someip::make<ServiceDescription>();
    task.copyTo(service);

    addUnsubscribe(service);
}

void ServiceAnnouncer::releaseTask(ServiceAnnouncerTask& task)
{
    task.clear();
    _taskPool.release(&task);
}

void ServiceAnnouncer::addProvidedServices(uint64_t const now)
{
    _serviceManager.updateServices(now); // callback offer / stopOffer
}

void ServiceAnnouncer::addQueries() const
{
    uint64_t const now = getSystemTimeMs32Bit();
    _queryManager.updateQueries(now); // callback to ServiceAnnouncer::find()
}

void ServiceAnnouncer::find(ServiceDescription const& service)
{
    // find service
    resetIfFull(_messageBuilder.addFind(
        service.serviceId,
        service.instanceId,
        service.majorVersion,
        service.minorVersion,
        service.ttl));
}

void ServiceAnnouncer::offer(ServiceDescription const& service)
{
    ServiceDescription temp(service);
    addOffer(temp);
}

void ServiceAnnouncer::addOffer(ServiceDescription& service)
{
    if (containsEventGroup(service))
    {
        return; // eventgroups are not announced, only services
    }

    service.ipAddress = _network.getLocalIp();
    resetIfFull(_messageBuilder.addOffer(
        service.serviceId,
        service.instanceId,
        service.majorVersion,
        service.minorVersion,
        service.ttl,
        service.ipAddress,
        service.port,
        service.proto));
}

void ServiceAnnouncer::stopOffer(ServiceDescription const& service)
{
    ServiceDescription temp(service);
    addStopOffer(temp);
}

void ServiceAnnouncer::addStopOffer(ServiceDescription& service)
{
    if (containsEventGroup(service))
    {
        return; // eventgroups are not announced, only services
    }

    service.ipAddress = _network.getLocalIp();
    resetIfFull(_messageBuilder.addDenounce(
        service.serviceId,
        service.instanceId,
        service.majorVersion,
        service.minorVersion,
        service.ttl,
        service.ipAddress,
        service.port,
        service.proto));
}

void ServiceAnnouncer::addSubscribe(ServiceDescription& service)
{
    service.ipAddress = _network.getLocalIp();
    resetIfFull(_messageBuilder.addSubscribe(
        service.serviceId,
        service.instanceId,
        service.eventGroup,
        service.majorVersion,
        service.ttl,
        service.ipAddress,
        service.port,
        service.proto));
}

void ServiceAnnouncer::addUnsubscribe(ServiceDescription& service)
{
    service.ipAddress = _network.getLocalIp();
    resetIfFull(_messageBuilder.addUnsubscribe(
        service.serviceId,
        service.instanceId,
        service.eventGroup,
        service.majorVersion,
        service.ttl,
        service.ipAddress,
        service.port,
        service.proto));
}

void ServiceAnnouncer::addSubscribeAck(ServiceDescription const& service)
{
    resetIfFull(_messageBuilder.addSubscribeAck(
        service.serviceId,
        service.instanceId,
        service.eventGroup,
        service.majorVersion,
        service.minorVersion,
        service.ttl));
}

void ServiceAnnouncer::addSubscribeAckMulticast(ServiceDescription const& service)
{
    resetIfFull(_messageBuilder.addSubscribeAckMulticast(
        service.serviceId,
        service.instanceId,
        service.eventGroup,
        service.majorVersion,
        service.minorVersion,
        service.ttl,
        service.ipAddress,
        service.port,
        service.proto));
}

void ServiceAnnouncer::addSubscribeNack(ServiceDescription const& service)
{
    resetIfFull(_messageBuilder.addSubscribeNack(
        service.serviceId,
        service.instanceId,
        service.eventGroup,
        service.majorVersion,
        service.minorVersion,
        service.ttl));
}

void ServiceAnnouncer::resetIfFull(SdMessageReturnCode const returnCode)
{
    if (returnCode == SdMessageReturnCode::SD_MESSAGE_IS_FULL)
    {
        finalizeMessage();
        /*
         * No need to check the result of 'startMessage' if we know that provided message buffer
         * is enough to contain at least header at ServiceAnnouncer construction stage.
         * Message buffer is declared as a static array.
         */
        (void)_messageBuilder.startMessage(_messageBuffer);
    }
}

void ServiceAnnouncer::sendMessage()
{
    auto const portResult = _network.getSdPort();

    if (!portResult.has_value())
    {
        WARN_LOG(SOMEIP, "ServiceAnnouncer::sendMessage() no SD port available");
        return;
    }

    uint16_t const port = portResult.value();

    auto channel = _network.getSdChannel(port, ::ip::IPEndpoint(_destinationAddress, port));

    if (!channel.has_value())
    {
        WARN_LOG(SOMEIP, "ServiceAnnouncer::sendMessage() no channel");
        return;
    }

    uint32_t const length = readTotalLength(_messageBuffer);
    memcpy(channel->getOutputBuffer().data(), _messageBuffer.data(), length);

    if (!channel->send(length))
    {
        WARN_LOG(SOMEIP, "ServiceAnnouncer::sendMessage() send failed");
        return;
    }

    Statistics::incCounter(Statistics::Counter::SD_FRAME_TX);
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
