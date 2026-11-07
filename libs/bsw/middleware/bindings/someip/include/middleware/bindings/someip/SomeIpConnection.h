/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/bindings/someip/IServiceEndpoint.h"
#include "middleware/bindings/someip/ISomeIpConnection.h"
#include "middleware/core/DatabaseManipulator.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/TransceiverBase.h"
#include "middleware/core/TransceiverContainer.h"
#include "middleware/core/types.h"

#include <etl/array.h>
#include <etl/error_handler.h>
#include <etl/vector.h>

#include <cstddef>
#include <cstdint>

namespace middleware::bindings::someip
{

/**
 * The configuration of one middleware cluster connection whose services travel
 * over SOME/IP instead of shared memory. It is installed on the connection from
 * the cluster of this node's skeletons and proxies to a cluster standing in for
 * the remote side, and routes each message to the endpoint of its service.
 *
 * \tparam MaxEndpoints     how many service endpoints it carries: one per
 *                          service, or per instance of a service.
 * \tparam MaxTransceivers  how many skeletons, and how many proxies, one service
 *                          may register on it.
 */
template<uint8_t MaxEndpoints, uint8_t MaxTransceivers>
class SomeIpConnection : public ISomeIpConnection
{
public:
    using Base    = ::middleware::core::IClusterConnectionConfiguration;
    using Message = ::middleware::core::Message;
    using HRESULT = ::middleware::core::HRESULT;

    SomeIpConnection(uint8_t const sourceClusterId, uint8_t const targetClusterId)
    : _sourceClusterId(sourceClusterId), _targetClusterId(targetClusterId)
    {}

    /**
     * Adds the endpoint of one service. Called by the endpoint's constructor, before
     * the connection is installed and before any skeleton or proxy is initialized.
     */
    void add(internal::IServiceEndpoint& endpoint) override
    {
        if (_endpoints.full())
        {
            ETL_ASSERT_FAIL(
                ETL_ERROR_GENERIC("SomeIpConnection: endpoint table full, raise MaxEndpoints"));
            return;
        }
        _endpoints.push_back(&endpoint);

        // Instances of one service share its registrations.
        uint16_t const serviceId = endpoint.getServiceId();
        if (::middleware::core::meta::DbManipulator::getTransceiversByServiceId(
                _proxyTransceivers.begin(), _proxyTransceivers.end(), serviceId)
            == _proxyTransceivers.end())
        {
            size_t const index = _proxyTransceivers.size();
            _proxyTransceivers.emplace_back(&_proxyLists[index], serviceId, 0U);
            _skeletonTransceivers.emplace_back(&_skeletonLists[index], serviceId, 0U);
        }
    }

    /** \see IClusterConnectionConfigurationBase::getSourceClusterId() */
    uint8_t getSourceClusterId() const override { return _sourceClusterId; }

    /** \see IClusterConnectionConfigurationBase::getTargetClusterId() */
    uint8_t getTargetClusterId() const override { return _targetClusterId; }

    /** \see IClusterConnectionConfigurationBase::registeredTransceiversCount() */
    size_t registeredTransceiversCount(uint16_t const serviceId) const override
    {
        return ::middleware::core::meta::DbManipulator::registeredTransceiversCount(
                   _proxyTransceivers.begin(), _proxyTransceivers.end(), serviceId)
               + ::middleware::core::meta::DbManipulator::registeredTransceiversCount(
                   _skeletonTransceivers.begin(), _skeletonTransceivers.end(), serviceId);
    }

    /**
     * A proxy's subscription is what starts SD FindService and Subscribe for the
     * instance it consumes, on the endpoint that consumes it.
     */
    HRESULT
    subscribe(::middleware::core::ProxyBase& proxy, uint16_t const serviceInstanceId) override
    {
        internal::IServiceEndpoint* const endpoint
            = consumerOf(proxy.getServiceId(), serviceInstanceId);
        if (endpoint == nullptr)
        {
            return HRESULT::InstanceNotFound;
        }
        HRESULT const result = ::middleware::core::meta::DbManipulator::subscribe(
            _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, serviceInstanceId);
        if ((result == HRESULT::Ok) && !endpoint->startQuerying())
        {
            ::middleware::core::meta::DbManipulator::unsubscribe(
                _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, proxy.getServiceId());
            return HRESULT::TransceiverInitializationFailed;
        }
        return result;
    }

    /** \see IClusterConnectionConfiguration::unsubscribe() */
    void unsubscribe(::middleware::core::ProxyBase& proxy, uint16_t const serviceId) override
    {
        ::middleware::core::meta::DbManipulator::unsubscribe(
            _proxyTransceivers.begin(), _proxyTransceivers.end(), proxy, serviceId);
        internal::IServiceEndpoint* const endpoint = consumerOf(serviceId, proxy.getInstanceId());
        if (endpoint != nullptr)
        {
            endpoint->stopQuerying();
        }
    }

    /** \see IClusterConnectionConfiguration::subscribe() */
    HRESULT
    subscribe(::middleware::core::SkeletonBase& skeleton, uint16_t const serviceInstanceId) override
    {
        return ::middleware::core::meta::DbManipulator::subscribe(
            _skeletonTransceivers.begin(),
            _skeletonTransceivers.end(),
            skeleton,
            serviceInstanceId);
    }

    /** \see IClusterConnectionConfiguration::unsubscribe() */
    void unsubscribe(::middleware::core::SkeletonBase& skeleton, uint16_t const serviceId) override
    {
        ::middleware::core::meta::DbManipulator::unsubscribe(
            _skeletonTransceivers.begin(), _skeletonTransceivers.end(), skeleton, serviceId);
    }

    /** No middleware-side timeouts: the stack owns SOME/IP request timeouts. */
    void registerTimeoutTransceiver(::middleware::core::ITimeoutHandler&) override {}

    /** \see ITimeoutConfiguration::unregisterTimeoutTransceiver() */
    void unregisterTimeoutTransceiver(::middleware::core::ITimeoutHandler&) override {}

    /** Nothing is ever registered, so there is nothing to update. */
    void updateTimeouts() override {}

    /**
     * Sends one middleware message to the wire, through the endpoint of its
     * service. The connection is the terminal consumer of everything it is
     * handed, so it frees the payload: sendMessage() deallocates only on its
     * same-cluster path, and replacing the SHM queue removes the receiver that
     * would otherwise do it.
     */
    bool write(Message const& msg) const override
    {
        internal::IServiceEndpoint* const endpoint
            = endpointOf(msg.getHeader().serviceId, msg.getHeader().serviceInstanceId);
        bool const isHandled = (endpoint != nullptr) && endpoint->send(msg);
        ::middleware::core::MessagePayloadBuilder::deallocate(msg);
        return isHandled;
    }

    /** \see IClusterConnectionConfigurationBase::dispatchMessage() */
    HRESULT dispatchMessage(Message const& msg) const override
    {
        return Base::dispatchMessage(
            _proxyTransceivers.begin(),
            _proxyTransceivers.end(),
            _skeletonTransceivers.begin(),
            _skeletonTransceivers.end(),
            msg);
    }

private:
    /** The endpoint of \p serviceId that provides or consumes \p instanceId. */
    internal::IServiceEndpoint*
    endpointOf(uint16_t const serviceId, uint16_t const instanceId) const
    {
        for (internal::IServiceEndpoint* const endpoint : _endpoints)
        {
            if ((endpoint->getServiceId() == serviceId)
                && ((endpoint->getProvidedInstanceId() == instanceId)
                    || (endpoint->getConsumedInstanceId() == instanceId)))
            {
                return endpoint;
            }
        }
        return nullptr;
    }

    /** The endpoint of \p serviceId that consumes \p instanceId. */
    internal::IServiceEndpoint*
    consumerOf(uint16_t const serviceId, uint16_t const instanceId) const
    {
        for (internal::IServiceEndpoint* const endpoint : _endpoints)
        {
            if ((endpoint->getServiceId() == serviceId)
                && (endpoint->getConsumedInstanceId() == instanceId))
            {
                return endpoint;
            }
        }
        return nullptr;
    }

    uint8_t _sourceClusterId;
    uint8_t _targetClusterId;
    ::etl::vector<internal::IServiceEndpoint*, MaxEndpoints> _endpoints{};

    // One list per service and direction, pointed at by that service's container.
    ::etl::array<::etl::vector<::middleware::core::TransceiverBase*, MaxTransceivers>, MaxEndpoints>
        _proxyLists{};
    ::etl::array<::etl::vector<::middleware::core::TransceiverBase*, MaxTransceivers>, MaxEndpoints>
        _skeletonLists{};
    ::etl::vector<::middleware::core::meta::TransceiverContainer, MaxEndpoints>
        _proxyTransceivers{};
    ::etl::vector<::middleware::core::meta::TransceiverContainer, MaxEndpoints>
        _skeletonTransceivers{};
};

} // namespace middleware::bindings::someip
