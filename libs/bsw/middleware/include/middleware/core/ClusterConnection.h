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

#include <etl/type_traits.h>

namespace middleware::core
{
class ProxyBase;
class SkeletonBase;

/**
 * Cluster connection for bidirectional communication with timeout support.
 * This class provides a cluster connection that supports both proxy and skeleton
 * subscriptions, with timeout management capabilities. It enables full bidirectional
 * communication between clusters with timeout tracking.
 */
class ClusterConnectionBidirectional final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionBidirectional(
        IClusterConnectionConfigurationBidirectional& configuration);

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) final;

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) final;
};

/**
 * Cluster connection for proxy-only communication with timeout support.
 * This class provides a cluster connection that only supports proxy subscriptions,
 * with timeout management capabilities. Skeleton subscriptions are not implemented.
 */
class ClusterConnectionProxyOnly final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionProxyOnly(IClusterConnectionConfigurationProxyOnly& configuration);

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /** Not supported: this is a proxy-only connection. */
    HRESULT subscribe(SkeletonBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /** No-op: this is a proxy-only connection. */
    void unsubscribe(SkeletonBase&, uint16_t const) final {}
};

/**
 * Cluster connection for skeleton-only communication with timeout support.
 * This class provides a cluster connection that only supports skeleton subscriptions,
 * with timeout management capabilities. Proxy subscriptions are not implemented.
 */
class ClusterConnectionSkeletonOnly final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionSkeletonOnly(
        IClusterConnectionConfigurationSkeletonOnly& configuration);

    /** Not supported: this is a skeleton-only connection. */
    HRESULT subscribe(ProxyBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(SkeletonBase&, uint16_t const) final;

    /** No-op: this is a skeleton-only connection. */
    void
    unsubscribe([[maybe_unused]] ProxyBase& proxy, [[maybe_unused]] uint16_t const serviceId) final
    {}

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(SkeletonBase&, [[maybe_unused]] uint16_t const) final;
};

/**
 * Type selector for cluster connection implementations.
 * This template struct selects the appropriate cluster connection type based on the
 * configuration type provided. It uses SFINAE (Substitution Failure Is Not An Error) with
 * enable_if to select the correct specialization. Instantiation without a valid configuration
 * type will lead to a compilation error by design.
 *
 * \tparam T the configuration type
 * \tparam Specialization SFINAE enabler parameter
 */
template<typename T, typename Specialization = void>
struct ClusterConnectionTypeSelector;

/**
 * Type selector specialization for proxy-only configurations.
 * \tparam T the proxy-only with configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename ::etl::enable_if<
        ::etl::is_base_of<IClusterConnectionConfigurationProxyOnly, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionProxyOnly;
};

/**
 * Type selector specialization for skeleton-only configurations.
 * \tparam T the skeleton-only with configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename ::etl::enable_if<
        ::etl::is_base_of<IClusterConnectionConfigurationSkeletonOnly, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionSkeletonOnly;
};

/**
 * Type selector specialization for bidirectional configurations.
 * \tparam T the bidirectional configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename ::etl::enable_if<
        ::etl::is_base_of<IClusterConnectionConfigurationBidirectional, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionBidirectional;
};

} // namespace middleware::core
