/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "IClusterConnectionConfigurationBase.h"
#include "middleware/core/ClusterConnectionBase.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"

namespace middleware::core
{
class ProxyBase;
class SkeletonBase;

/**
 * Unified cluster connection with timeout support.
 *
 * This class forwards proxy/skeleton subscribe operations to a unified configuration
 * object, independent from the underlying connection topology.
 */
class ClusterConnection final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnection(IClusterConnectionConfiguration& configuration);

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) final;

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) final;
};

} // namespace middleware::core
