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

#include "someip/IRpcReceiver.h"
#include "someip/IServiceAnnouncer.h"
#include "someip/IServiceListener.h"
#include "someip/ServiceQuery.h"
#include "someip/TcpClientChannelValidator.h"

#include <ip/IPEndpoint.h>

#include <etl/flat_set.h>
#include <cstdint>

namespace someip
{
/**
 * Responsible to maintain a list of service queries.
 */
class QueryManager
{
public:
    void wire(IServiceAnnouncer* serviceAnnouncer, IRpcReceiver* rpcReceiver);

    /**
     * Initialize registered queries.
     */
    void start() const;

    /**
     * Update status of all queries to unavailable and send stop subscribes.
     */
    void stop() const;

    /**
     * Register a query.
     *
     * \post query will be initialized.
     *
     * \return true if query was registered.
     */
    bool registerQuery(ServiceQuery& query);

    /**
     * Unregister a query.
     *
     * \post query will be set to idle.
     *
     * \return true if query was removed.
     */
    bool unregisterQuery(ServiceQuery& query);

    /**
     * Answers if given query is registered.
     */
    bool hasQuery(ServiceQuery& query) const;

    /**
     * Returns query if registered or nullptr otherwise.
     */
    ServiceQuery const* getQuery(service_id::type serviceId, instance_id::type instanceId) const;

    /**
     * Check the presence of a local port in any eventgroup for given serviceId,
     * instanceId, majorVersion.
     *
     * \return true if a query with the specified port was found.
     */
    bool isEventgroupPort(
        service_id::type serviceId,
        instance_id::type instanceId,
        major_version::type majorVersion,
        uint16_t port) const;

    /**
     * Answers if there is a query registered with a given ServiceDescription.
     */
    bool hasServiceDescription(ServiceDescription const& service) const;

    /**
     * Update queries depending on service status change.
     *
     * \post query-listener will get notified on service status.
     */
    void
    updateQueries(ServiceDescription const& service, IServiceListener::ServiceStatus status) const;

    /**
     * Update queries depending on time change.
     *
     * \post service-announcer will get notified on findings.
     */
    void updateQueries(uint64_t timestamp) const;

    /**
     * Handle a received subscribe ack.
     *
     * \post rpc-receiver will get notified on multicast reception.
     */
    void subscribeAckReceived(
        ServiceDescription const& service,
        ::ip::IPEndpoint const& multicastEndpoint,
        ::ip::IPAddress const& sourceAddress);

    /**
     * Handle a received subscribe nack.
     *
     * \post query-listener will get notified on service status.
     */
    void
    subscribeNackReceived(ServiceDescription const& service, ::ip::IPAddress const& sourceAddress);

    /**
     * Handle a received offer.
     *
     * \post service-announcer will get notified on subscribe / unsubscribe.
     */
    void offerReceived(ServiceDescription const& service, ::ip::IPAddress const& sourceAddress);

    /**
     * Handle a received stop offer.
     */
    void stopOfferReceived(ServiceDescription const& service) const;

    uint32_t getMaxNumQueries() const { return static_cast<uint32_t>(_queries.max_size()); }

    uint32_t getNumQueries() const { return static_cast<uint32_t>(_queries.size()); }

protected:
    struct ServiceIdComparator
    {
        bool operator()(ServiceQuery const* lhs, uint16_t rhs) const;
    };

    struct LessThanComparator
    {
        bool operator()(ServiceQuery const* lhs, ServiceQuery const* rhs) const;
    };

    using QueryList = ::etl::iflat_set<ServiceQuery*, LessThanComparator>;

    QueryManager(QueryList& queryList, TcpClientChannelValidator& validator);

private:
    void updateQueryInInitialPhase(uint64_t timestamp, ServiceQuery& query) const;
    void updateQueryInRepetitionPhase(uint64_t timestamp, ServiceQuery& query) const;

    QueryList& _queries;

    IServiceAnnouncer* _pServiceAnnouncer;
    IRpcReceiver* _pRpcReceiver;
    TcpClientChannelValidator& _tcpClientChannelValidator;
};

namespace declare
{
template<uint16_t NUM_QUERIES>
class QueryManager : public ::someip::QueryManager
{
public:
    explicit QueryManager(TcpClientChannelValidator& tcpClientChannelValidator)
    : ::someip::QueryManager(_queries, tcpClientChannelValidator)
    {}

private:
    ::etl::flat_set<ServiceQuery*, NUM_QUERIES, LessThanComparator> _queries;
};

} // namespace declare
} // namespace someip
