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

#include "someip/IServiceTrackerListener.h"
#include "someip/ServiceDescription.h"
#include "someip/ServiceQuery.h"

#include <ip/IPAddress.h>

#include <etl/flat_set.h>
#include <cstdint>

namespace someip
{
namespace internal
{
struct TrackedService
{
    ServiceDescription serviceDescription;
    uint32_t sequentialOffersReceived;
    uint64_t lastOfferTimestamp;
};

struct LessThanComparator
{
    bool operator()(TrackedService const& lhs, TrackedService const& rhs) const;
};

class FindServiceCondition
{
public:
    explicit FindServiceCondition(ServiceDescription const& service, bool instanceAny = false);

    bool isValid() const;
    bool operator()(TrackedService const& service) const;

private:
    ServiceDescription const& _service;
    bool _instanceAny;
};

class FindInstanceCondition
{
public:
    explicit FindInstanceCondition(ServiceDescription const& service);

    bool isValid() const;
    bool operator()(TrackedService const& service) const;

private:
    ServiceDescription const& _service;
};

class RemoveIpCondition
{
public:
    RemoveIpCondition(::ip::IPAddress const& ipAddr, IServiceTrackerListener* listener);

    bool operator()(TrackedService const& service) const;

private:
    ::ip::IPAddress const& _ip;
    IServiceTrackerListener* _pListener;
};

class UpdateTTLCondition
{
public:
    UpdateTTLCondition(uint32_t ticks, IServiceTrackerListener* listener);

    bool operator()(TrackedService const& service) const;

private:
    uint32_t const _ticks;
    IServiceTrackerListener* _pListener;
};

} // namespace internal

/**
 * Responsible to maintain a list of remote services.
 */
class ServiceTracker
{
public:
    struct ServiceReliabilityConfig
    {
        uint32_t expectedOfferPeriodMs;
        uint32_t offerCountThreshold;
    };

    void init(IServiceTrackerListener& listener);

    /**
     * Add or update a service identified by:
     *
     * - serviceId
     * - majorVersion
     * - instanceId
     *
     * \note service TTL > 0 must be set.
     *
     * \post on success the service was added or internal values were updated.
     *
     * \return true if successful.
     */
    bool addService(ServiceDescription const& service);

    /**
     * Remove a service identified by:
     *
     * - serviceId
     * - majorVersion
     * - instanceId
     *
     * \post the service was removed if being contained.
     */
    void removeService(ServiceDescription const& service);

    /**
     * Remove all tracked services:
     */
    void stop();

    /**
     * Get a service identified by:
     *
     * - serviceId
     * - majorVersion
     * - instanceId
     *
     * \post on success the provided service was updated with internal values.
     *
     * \return true if service found.
     */
    bool getService(ServiceDescription& service) const;

    /**
     * Find instanceId of a service identified by:
     *
     * - serviceId
     * - majorVersion
     * - ipAddress
     * - port
     *
     * \return instanceId or instance_id::any if not found.
     */
    instance_id::type getInstanceId(ServiceDescription const& service) const;

    /**
     * Adjust TTL = 0 and remove services associated with this IP.
     */
    void rebootDetected(::ip::IPAddress const& ipAddr);

    /**
     * Adjust TTL -= ticks of all services and remove those with TTL == 0.
     */
    void updateTTLs(uint32_t ticks);

    /**
     * Notifies a service query if a service is contained matching:
     *
     * - serviceId
     * - majorVersion
     * - instanceId
     *
     * \note If instanceId == INSTANCE_ID_ANY all corresponding services will be notify.
     */
    void notifyServices(ServiceQuery& query) const;

    uint16_t getCurrentNumberOfServices() const;
    uint16_t getMaximumNumberOfServices() const;

protected:
    using ServiceList = ::etl::iflat_set<internal::TrackedService, internal::LessThanComparator>;

    explicit ServiceTracker(ServiceList& serviceList);
    ServiceTracker(ServiceList& serviceList, ServiceReliabilityConfig const& reliabilityConfig);

    ~ServiceTracker() = default;

private:
    ServiceList& _services;
    IServiceTrackerListener* _pListener;
    ServiceReliabilityConfig _reliabilityConfig;
};

namespace declare
{

template<uint16_t NUM_SERVICES>
class ServiceTracker : public ::someip::ServiceTracker
{
public:
    ServiceTracker();
    explicit ServiceTracker(ServiceTracker::ServiceReliabilityConfig const& reliabilityConfig);

private:
    ::etl::flat_set<internal::TrackedService, NUM_SERVICES, internal::LessThanComparator>
        _serviceList;
};

template<uint16_t NUM_SERVICES>
inline ServiceTracker<NUM_SERVICES>::ServiceTracker()
: ::someip::ServiceTracker(_serviceList), _serviceList()
{}

template<uint16_t NUM_SERVICES>
inline ServiceTracker<NUM_SERVICES>::ServiceTracker(
    ServiceTracker::ServiceReliabilityConfig const& reliabilityConfig)
: ::someip::ServiceTracker(_serviceList, reliabilityConfig), _serviceList()
{}

} // namespace declare
} // namespace someip
