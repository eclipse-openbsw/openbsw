/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/ClusterConnection.h"

#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/types.h"

namespace middleware::core
{

ClusterConnectionBidirectional::ClusterConnectionBidirectional(
    IClusterConnectionConfigurationBidirectional& configuration)
: Base(configuration)
{}

HRESULT
ClusterConnectionBidirectional::subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

void ClusterConnectionBidirectional::unsubscribe(ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

HRESULT
ClusterConnectionBidirectional::subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnectionBidirectional::unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

ClusterConnectionProxyOnly::ClusterConnectionProxyOnly(
    IClusterConnectionConfigurationProxyOnly& configuration)
: Base(configuration)
{}

HRESULT
ClusterConnectionProxyOnly::subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationProxyOnly&>(Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

void ClusterConnectionProxyOnly::unsubscribe(ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationProxyOnly&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

ClusterConnectionSkeletonOnly::ClusterConnectionSkeletonOnly(
    IClusterConnectionConfigurationSkeletonOnly& configuration)
: Base(configuration)
{}

HRESULT
ClusterConnectionSkeletonOnly::subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationSkeletonOnly&>(Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnectionSkeletonOnly::unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationSkeletonOnly&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

} // namespace middleware::core
