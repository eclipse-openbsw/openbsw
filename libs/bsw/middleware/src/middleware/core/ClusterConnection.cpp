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

ClusterConnection::ClusterConnection(IClusterConnectionConfiguration& configuration)
: Base(configuration)
{}

HRESULT ClusterConnection::subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfiguration&>(Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

void ClusterConnection::unsubscribe(ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfiguration&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

HRESULT ClusterConnection::subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfiguration&>(Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnection::unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfiguration&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

} // namespace middleware::core
