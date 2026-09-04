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

#include "someip/ProvidedService.h"

#include <etl/flat_set.h>
#include <cstdint>

namespace someip
{
class IServiceAnnouncer;
class ServiceAnnouncerTask;

/**
 * Responsible to maintain a list of provided services.
 */
class ServiceManager
{
public:
    enum class FindServiceResult : uint8_t
    {
        FIND_SERVICE_OK,
        FIND_SERVICE_UNKNOWN,
        FIND_SERVICE_WRONG_MAJOR_VERSION
    };

    void wire(IServiceAnnouncer* serviceAnnouncer);

    /**
     * Initialize registered services.
     */
    void start();

    /**
     * Shutdown registered services.
     */
    void stop();

    /**
     * Register a service.
     *
     * \post service will be initialized.
     *
     * \return true if service was registered.
     */
    bool registerService(ProvidedService& service);

    /**
     * Unregister a service.
     *
     * \post service will be removed asynchronous.
     *
     * \return true if service was removed.
     */
    bool unregisterService(ProvidedService& service);

    /**
     * Answers if given service is registered.
     */
    bool hasService(ProvidedService& service) const;

    /**
     * Answers if there is a service/eventgroup registered with a given ServiceDescription.
     */
    bool hasServiceDescription(ServiceDescription const& service) const;

    /**
     * Runs lifecycle on services
     */
    void updateServices(uint64_t time);

    /**
     * Find the instance ID of a service based on the given ServiceDescription.
     *
     * The search is performed based on:
     * - serviceId
     * - majorVersion
     * - port
     *
     * \return The instance ID of the service if found, or instance_id::any
     * if not found.
     */
    instance_id::type getInstanceId(ServiceDescription const& service) const;

    /**
     * Find a service identified by:
     *
     * - serviceId
     * - instanceId
     * - majorVersion
     *
     * \return service pointer if successful, or 0L otherwise.
     */
    ProvidedService const* getService(ServiceDescription const& service) const;

    /**
     * Find a eventgroup identified by:
     *
     * - serviceId
     * - instanceId
     * - majorVersion
     * - eventgroupId
     *
     * \return service pointer if successful, or 0L otherwise.
     */
    ProvidedService const* getEventGroup(ServiceDescription const& service) const;

    /**
     * Find a handler identified by:
     *
     * - serviceId
     * - majorVersion
     * - port
     * - proto
     *
     * \post result of search will be set.
     *
     * \return handler pointer if successful, or 0L otherwise.
     */
    ServiceHandler* getHandler(ServiceDescription const& service, FindServiceResult& result) const;

    /**
     * Trigger offers for a service-announcer task.
     */
    void triggerOffers(ServiceAnnouncerTask const& task) const;

    /**
     * Trigger stop offers after being stopped.
     *
     * \note should be called after stop,- calls stop implicitly otherwise.
     */
    void triggerStopOffers();

    uint16_t getNumberOfServices() const;
    uint16_t getMaxNumberOfServices() const;

protected:
    struct ServiceIdComparator
    {
        bool operator()(ProvidedService const* lhs, uint16_t rhs) const;
    };

    struct LessThanComparator
    {
        bool operator()(ProvidedService const* lhs, ProvidedService const* rhs) const;
    };

    using ServiceList = ::etl::iflat_set<ProvidedService*, LessThanComparator>;

    explicit ServiceManager(ServiceList& serviceList);

private:
    void removeServices();

    ServiceList& _services;
    IServiceAnnouncer* _pServiceAnnouncer;

    bool _stopCalled;
};

namespace declare
{
template<uint16_t NUM_SERVICES>
class ServiceManager : public ::someip::ServiceManager
{
public:
    ServiceManager();

private:
    ::etl::flat_set<ProvidedService*, NUM_SERVICES, LessThanComparator> _services;
};

template<uint16_t NUM_SERVICES>
inline ServiceManager<NUM_SERVICES>::ServiceManager()
: ::someip::ServiceManager(_services), _services()

{}

} // namespace declare
} // namespace someip
