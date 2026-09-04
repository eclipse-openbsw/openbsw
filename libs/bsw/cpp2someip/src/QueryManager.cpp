/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/QueryManager.h"

#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"
#include "someip/init.h"
#include "someip/logger.h"

#include <etl/algorithm.h>
#include <etl/intrusive_forward_list.h>
#include <etl/intrusive_links.h>
#include <algorithm>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

QueryManager::QueryManager(QueryList& queryList, TcpClientChannelValidator& validator)
: _queries(queryList)
, _pServiceAnnouncer(nullptr)
, _pRpcReceiver(nullptr)
, _tcpClientChannelValidator(validator)
{}

void QueryManager::wire(IServiceAnnouncer* const serviceAnnouncer, IRpcReceiver* const rpcReceiver)
{
    _pServiceAnnouncer = serviceAnnouncer;
    _pRpcReceiver      = rpcReceiver;
}

void QueryManager::start() const
{
    if (_pServiceAnnouncer == nullptr)
    {
        WARN_LOG(SOMEIP, "QueryManager::start() no service-announcer");
    }
    if (_pRpcReceiver == nullptr)
    {
        WARN_LOG(SOMEIP, "QueryManager::start() no rpc-receiver");
    }

    for (ServiceQuery* query : _queries)
    {
        if (query != nullptr)
        {
            ::someip::init(*query);
        }
    }
}

void QueryManager::stop() const
{
    for (ServiceQuery* query : _queries)
    {
        IServiceListener* const listener = query->listener;

        if (listener != nullptr)
        {
            listener->serviceStatusChanged(
                query->description, IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE);
        }
        else
        {
            if (_pServiceAnnouncer != nullptr)
            {
                _pServiceAnnouncer->unsubscribe(query->description, query->serviceDiscoveryAddress);
            }
        }
    }
}

bool QueryManager::registerQuery(ServiceQuery& query)
{
    if (hasQuery(query))
    {
        return false;
    }

    if (_queries.full())
    {
        WARN_LOG(
            SOMEIP,
            "QueryManager full! Rejecting query (service: %d, version: %d, instance: %d, "
            "eventgroup: %d)",
            query.description.serviceId,
            query.description.majorVersion,
            query.description.instanceId,
            query.description.eventGroup);
        return false;
    }

    INFO_LOG(
        SOMEIP,
        "QueryManager::registerQuery(service: %d, version: %d, instance: %d, eventgroup: %d)",
        query.description.serviceId,
        query.description.majorVersion,
        query.description.instanceId,
        query.description.eventGroup);

    (void)_queries.insert(&query);
    ::someip::init(query);

    return true;
}

bool QueryManager::unregisterQuery(ServiceQuery& query)
{
    if (!hasQuery(query))
    {
        return false;
    }

    INFO_LOG(
        SOMEIP,
        "QueryManager::unregisterQuery(service: %d, version: %d, instance: %d, eventgroup: %d)",
        query.description.serviceId,
        query.description.majorVersion,
        query.description.instanceId,
        query.description.eventGroup);

    if (containsEventGroup(query.description)
        && (query.subscriptionState != ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED))
    {
        if (_pServiceAnnouncer != nullptr)
        {
            _pServiceAnnouncer->unsubscribe(query.description, query.serviceDiscoveryAddress);
        }

        query.subscriptionState = ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED;
    }

    auto it = _queries.find(&query);
    if (it != _queries.end())
    {
        _queries.erase(it);
    }
    query.state = ServiceQuery::ServiceQueryState::QUERY_IDLE_PHASE;

    return true;
}

bool QueryManager::hasQuery(ServiceQuery& query) const { return _queries.contains(&query); }

bool QueryManager::isEventgroupPort(
    service_id::type const serviceId,
    instance_id::type const instanceId,
    major_version::type const majorVersion,
    uint16_t const port) const
{
    for (ServiceQuery* query : _queries)
    {
        auto service         = ::someip::make<ServiceDescription>();
        service.serviceId    = serviceId;
        service.instanceId   = instanceId;
        service.majorVersion = majorVersion;
        if ((query != nullptr) && isEventgroupOfService(query->description, service))
        {
            if (port == query->description.port)
            {
                return true;
            }
        }
    }
    return false;
}

ServiceQuery const*
QueryManager::getQuery(service_id::type const serviceId, instance_id::type const instanceId) const
{
    QueryList::const_iterator queryItr = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [serviceId](ServiceQuery const* query)
        { return query && query->description.serviceId >= serviceId; });
    for (; queryItr != _queries.end(); ++queryItr)
    {
        ServiceQuery* const query = *queryItr;

        if ((query != nullptr) && (query->description.serviceId != serviceId))
        {
            break;
        }
        if ((query != nullptr) && (query->description.instanceId == instanceId))
        {
            return query;
        }
    }
    return nullptr;
}

bool QueryManager::hasServiceDescription(ServiceDescription const& service) const
{
    QueryList::const_iterator query = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [serviceId = service.serviceId](ServiceQuery const* q)
        { return q && q->description.serviceId >= serviceId; });
    for (; query != _queries.cend(); ++query)
    {
        if (query != nullptr)
        {
            if ((*query)->description.serviceId != service.serviceId)
            {
                break;
            }
            if (isInstanceOf(service, (*query)->description))
            {
                return true;
            }
        }
    }
    return false;
}

void QueryManager::updateQueries(
    ServiceDescription const& service, IServiceListener::ServiceStatus const status) const
{
    DEBUG_LOG(
        SOMEIP, "QueryManager::updateQueries(service: %d, status: %d)", service.serviceId, status);

    ::etl::intrusive_forward_list<IServiceListener, ::etl::forward_link<0>> listeners;

    QueryList::const_iterator queryItr = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [serviceId = service.serviceId](ServiceQuery const* q)
        { return q && q->description.serviceId >= serviceId; });
    for (; queryItr != _queries.end(); ++queryItr)
    {
        ServiceQuery* const query = *queryItr;
        if (query != nullptr)
        {
            if (query->description.serviceId != service.serviceId)
            {
                break;
            }
            // handle queries for services
            if (isInstanceOf(service, query->description))
            {
                if (IServiceListener::ServiceStatus::SERVICE_AVAILABLE == status)
                {
                    query->state = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;
                }
                // offer service ttl is expired
                else if (
                    (IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE == status)
                    && (service.ttl == 0U))
                {
                    query->state = ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE;
                }
                else
                {
                    // do nothing
                }

                IServiceListener* const listener = query->listener;

                if (listener != nullptr)
                {
                    query->serviceDiscoveryAddress = service.ipAddress;
                    listeners.push_front(*listener);
                }
            }
            // handle queries for eventgroups
            else if (isEventgroupOfService(query->description, service))
            {
                if (IServiceListener::ServiceStatus::SERVICE_UNAVAILABLE == status)
                {
                    query->subscriptionState = ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED;
                }
            }
            else
            {
                ; // nothing else to do
            }
        }
    }

    // notifying listeners may change queries
    while (!listeners.empty())
    {
        IServiceListener* const listener = &listeners.front();
        listeners.pop_front();

        listener->serviceStatusChanged(service, status);

        auto const endItr = _queries.cend();
        queryItr          = _queries.begin();
        for (; queryItr != endItr; ++queryItr)
        {
            ServiceQuery* const query = *queryItr;
            if (query != nullptr)
            {
                if (query->description.serviceId != service.serviceId)
                {
                    break;
                }
                if (isEventgroupOfService(query->description, service))
                {
                    listener->updateEventgroupDescription(query->description, status);
                }
            }
        }
    }
}

void QueryManager::updateQueryInInitialPhase(uint64_t const timestamp, ServiceQuery& query) const
{
    if (query.timestamp == 0U)
    {
        query.timestamp = timestamp; // start waiting
    }
    uint32_t const timePassed = static_cast<uint32_t>(timestamp - query.timestamp);
    if (timePassed >= query.sdConfig._initialDelay)
    {
        if ((_pServiceAnnouncer != nullptr) && (!containsEventGroup(query.description)))
        {
            _pServiceAnnouncer->find(query.description);
        }

        query.state           = ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE;
        query.timestamp       = timestamp;
        query.repetitionCount = 1U;
    }
}

void QueryManager::updateQueryInRepetitionPhase(uint64_t const timestamp, ServiceQuery& query) const
{
    uint32_t const timePassed = static_cast<uint32_t>(timestamp - query.timestamp);
    uint32_t const repetitionCount
        = etl::min(query.repetitionCount, static_cast<uint32_t>(sizeof(uint32_t) * 8U));
    uint32_t const repetitionDelay = (static_cast<uint32_t>(1U) << (repetitionCount - 1U))
                                     * query.sdConfig._repetitionsBaseDelay;
    if (timePassed >= repetitionDelay)
    {
        if ((_pServiceAnnouncer != nullptr) && (!containsEventGroup(query.description)))
        {
            _pServiceAnnouncer->find(query.description);
        }

        query.timestamp = timestamp;
        ++query.repetitionCount;

        if (query.repetitionCount >= query.sdConfig._repetitionsMax)
        {
            query.state = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE;
        }
    }
}

void QueryManager::updateQueries(uint64_t const timestamp) const
{
    for (ServiceQuery* query : _queries)
    {
        if (query != nullptr)
        {
            // should never happen, but safety first
            if (timestamp < query->timestamp)
            {
                continue;
            }

            if (ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE == query->state)
            {
                updateQueryInInitialPhase(timestamp, *query);
            }
            else if (ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE == query->state)
            {
                updateQueryInRepetitionPhase(timestamp, *query);
            }
        }
    }
}

void QueryManager::subscribeAckReceived(
    ServiceDescription const& service,
    ::ip::IPEndpoint const& multicastEndpoint,
    ::ip::IPAddress const& sourceAddress)
{
    QueryList::const_iterator queryItr = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [serviceId = service.serviceId](ServiceQuery const* q)
        { return q && q->description.serviceId >= serviceId; });
    for (; queryItr != _queries.end(); ++queryItr)
    {
        ServiceQuery* const query = *queryItr;
        if (query != nullptr)
        {
            if (query->description.serviceId != service.serviceId)
            {
                break;
            }
            if (((query->state == ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE)
                 || (query->state == ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE))
                && matches(query->description, service)
                && query->serviceDiscoveryAddress == sourceAddress)
            {
                query->subscriptionState = ServiceQuery::SubscriptionState::STATE_ACK_RECEIVED;

                if ((multicastEndpoint.isSet())
                    && (::ip::isMulticastAddress(multicastEndpoint.getAddress()))
                    && (!query->multicastAddress.isSet()))
                {
                    query->multicastAddress = multicastEndpoint;

                    if (_pRpcReceiver != nullptr)
                    {
                        (void)_pRpcReceiver->requestMulticastReception(multicastEndpoint);
                    }
                }
            }
        }
    }
}

void QueryManager::subscribeNackReceived(
    ServiceDescription const& service, ::ip::IPAddress const& sourceAddress)
{
    ServiceQuery sq = make<ServiceQuery>();
    sq.description  = service;
    LessThanComparator comp;
    QueryList::const_iterator queryItr = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [&sq, &comp](ServiceQuery const* query) { return !comp(query, &sq); });
    if ((service.proto == proto::SD_L4_PROTO_TCP) && (queryItr != _queries.cend())
        && (matches(service, (*queryItr)->description)))
    {
        ServiceQuery* const query = *queryItr;
        if ((nullptr != query) && query->serviceDiscoveryAddress == sourceAddress)
        {
            TcpClientChannelValidator::CachedValidator tcpValidator(_tcpClientChannelValidator);
            tcpValidator.checkClientChannel(
                ::ip::IPEndpoint(service.ipAddress, service.port), query->description.port);
        }
    }

    while ((queryItr != _queries.cend()) && service.serviceId == (*queryItr)->description.serviceId
           && service.instanceId == (*queryItr)->description.instanceId
           && (*queryItr)->serviceDiscoveryAddress == sourceAddress)
    {
        (*queryItr)->subscriptionState = ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED;
        if ((nullptr != (*queryItr)->listener))
        {
            (*queryItr)->listener->serviceStatusChanged(
                service, IServiceListener::ServiceStatus::SERVICE_SUBSCRIPTION_NACK);
        }
        ++queryItr;
    }
}

void QueryManager::offerReceived(
    ServiceDescription const& service, ::ip::IPAddress const& sourceAddress)
{
    ::etl::optional<TcpClientChannelValidator::CachedValidator> tcpValidator;
    if (service.proto == proto::SD_L4_PROTO_TCP)
    {
        tcpValidator.emplace(_tcpClientChannelValidator);
    }

    QueryList::const_iterator queryItr = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [serviceId = service.serviceId](ServiceQuery const* q)
        { return q && q->description.serviceId >= serviceId; });
    for (; queryItr != _queries.end(); ++queryItr)
    {
        ServiceQuery* const query = *queryItr;
        if (query != nullptr)
        {
            if (query->description.serviceId != service.serviceId)
            {
                break;
            }
            if (((query->state == ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE)
                 || (query->state == ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE))
                && isEventgroupOfService(query->description, service))
            {
                if ((ServiceQuery::SubscriptionState::STATE_UNSUBSCRIBED
                     == query->subscriptionState)
                    && (service.proto == proto::SD_L4_PROTO_TCP)
                    && (!tcpValidator->isChannelEstablished(
                        ::ip::IPEndpoint(service.ipAddress, service.port),
                        query->description.port)))
                {
                    // cannot subscribe without data channel
                    continue;
                }

                if (ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK
                    == query->subscriptionState)
                {
                    if (_pServiceAnnouncer != nullptr)
                    {
                        _pServiceAnnouncer->unsubscribe(
                            query->description, query->serviceDiscoveryAddress);
                    }
                }
                else
                {
                    // ServiceQuery::SubscriptionState::STATE_ACK_RECEIVED - subscription has been
                    // made, updating existing
                }
                query->subscriptionState = ServiceQuery::SubscriptionState::STATE_WAITING_FOR_ACK;
                query->description.ipAddress   = service.ipAddress;
                query->description.ttl         = service.ttl;
                query->serviceDiscoveryAddress = sourceAddress;

                if (_pServiceAnnouncer != nullptr)
                {
                    _pServiceAnnouncer->subscribe(
                        query->description, query->serviceDiscoveryAddress);
                }
            }
        }
    }
}

void QueryManager::stopOfferReceived(ServiceDescription const& service) const
{
    QueryList::const_iterator queryItr = etl::find_if(
        _queries.cbegin(),
        _queries.cend(),
        [serviceId = service.serviceId](ServiceQuery const* q)
        { return q && q->description.serviceId >= serviceId; });
    for (; queryItr != _queries.end(); ++queryItr)
    {
        ServiceQuery* const query = *queryItr;
        if (query != nullptr)
        {
            if (query->description.serviceId != service.serviceId)
            {
                break;
            }
            if (((query->state == ServiceQuery::ServiceQueryState::QUERY_INITIAL_WAIT_PHASE)
                 || (query->state == ServiceQuery::ServiceQueryState::QUERY_REPETITION_PHASE))
                && (!containsEventGroup(query->description))
                && matches(query->description, service))
            {
                query->state
                    = ServiceQuery::ServiceQueryState::QUERY_MAIN_PHASE; // don't send FINDs
                                                                         // after a StopOffer
            }
        }
    }
}

bool QueryManager::LessThanComparator::operator()(
    ServiceQuery const* const lhs, ServiceQuery const* const rhs) const
{
    if ((lhs != nullptr) && (rhs != nullptr))
    {
        uint16_t const lhsServiceId = lhs->description.serviceId;
        uint16_t const rhsServiceId = rhs->description.serviceId;
        if (lhsServiceId != rhsServiceId)
        {
            return lhsServiceId < rhsServiceId;
        }

        uint16_t const lhsInstanceId = lhs->description.instanceId;
        uint16_t const rhsInstanceId = rhs->description.instanceId;
        if (lhsInstanceId != rhsInstanceId)
        {
            return lhsInstanceId < rhsInstanceId;
        }

        uint16_t const lhsEventGroup = lhs->description.eventGroup;
        uint16_t const rhsEventGroup = rhs->description.eventGroup;
        if (lhsEventGroup != rhsEventGroup)
        {
            return lhsEventGroup < rhsEventGroup;
        }

        uint8_t const lhsMajorVersion = lhs->description.majorVersion;
        uint8_t const rhsMajorVersion = rhs->description.majorVersion;
        if (lhsMajorVersion != rhsMajorVersion)
        {
            return lhsMajorVersion < rhsMajorVersion;
        }

        return lhs->timestamp < rhs->timestamp;
    }
    return false;
}

bool QueryManager::ServiceIdComparator::operator()(
    ServiceQuery const* const lhs, service_id::type const rhs) const
{
    if (lhs != nullptr)
    {
        return lhs->description.serviceId < rhs;
    }

    return false;
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
