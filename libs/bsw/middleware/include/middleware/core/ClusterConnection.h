// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/ClusterConnectionBase.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"

#include <etl/type_traits.h>

namespace middleware
{
namespace core
{
class ProxyBase;
class SkeletonBase;

/**
 * Cluster connection for proxy-only communication without timeout support.
 * This class provides a cluster connection that only supports proxy subscriptions,
 * without timeout management. Skeleton subscriptions are not implemented and will return
 * NotImplemented.
 */
class ClusterConnectionNoTimeoutProxyOnly final : public ClusterConnectionBase
{
    using Base = ClusterConnectionBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionNoTimeoutProxyOnly(
        IClusterConnectionConfigurationProxyOnly& configuration);

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
 * Cluster connection for skeleton-only communication without timeout support.
 * This class provides a cluster connection that only supports skeleton subscriptions,
 * without timeout management. Proxy subscriptions are not implemented and will return
 * NotImplemented.
 */
class ClusterConnectionNoTimeoutSkeletonOnly final : public ClusterConnectionBase
{
    using Base = ClusterConnectionBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionNoTimeoutSkeletonOnly(
        IClusterConnectionConfigurationSkeletonOnly& configuration);

    /** Not supported: this is a skeleton-only connection. */
    HRESULT subscribe(ProxyBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /** \see IClusterConnection::subscribe() */
    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) final;

    /** No-op: this is a skeleton-only connection. */
    void unsubscribe(ProxyBase&, uint16_t const) final {}

    /** \see IClusterConnection::unsubscribe() */
    void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) final;
};

/**
 * Cluster connection for bidirectional communication without timeout support.
 * This class provides a cluster connection that supports both proxy and skeleton
 * subscriptions, without timeout management. It enables full bidirectional communication
 * between clusters.
 */
class ClusterConnectionNoTimeoutBidirectional final : public ClusterConnectionBase
{
    using Base = ClusterConnectionBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionNoTimeoutBidirectional(
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
 * Cluster connection for bidirectional communication with timeout support.
 * This class provides a cluster connection that supports both proxy and skeleton
 * subscriptions, with timeout management capabilities. It enables full bidirectional
 * communication between clusters with timeout tracking.
 */
class ClusterConnectionBidirectionalWithTimeout final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionBidirectionalWithTimeout(
        IClusterConnectionConfigurationBidirectionalWithTimeout& configuration);

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
class ClusterConnectionProxyOnlyWithTimeout final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionProxyOnlyWithTimeout(
        IClusterConnectionConfigurationProxyOnlyWithTimeout& configuration);

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
class ClusterConnectionSkeletonOnlyWithTimeout final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionSkeletonOnlyWithTimeout(
        IClusterConnectionConfigurationSkeletonOnlyWithTimeout& configuration);

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
 * \tparam T the proxy-only configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationProxyOnly, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionNoTimeoutProxyOnly;
};

/**
 * Type selector specialization for skeleton-only configurations.
 * \tparam T the skeleton-only configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationSkeletonOnly, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionNoTimeoutSkeletonOnly;
};

/**
 * Type selector specialization for bidirectional configurations.
 * \tparam T the bidirectional configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationBidirectional, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionNoTimeoutBidirectional;
};

/**
 * Type selector specialization for proxy-only configurations with timeout.
 * \tparam T the proxy-only with timeout configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationProxyOnlyWithTimeout, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionProxyOnlyWithTimeout;
};

/**
 * Type selector specialization for skeleton-only configurations with timeout.
 * \tparam T the skeleton-only with timeout configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationSkeletonOnlyWithTimeout, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionSkeletonOnlyWithTimeout;
};

/**
 * Type selector specialization for bidirectional configurations with timeout.
 * \tparam T the bidirectional with timeout configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationBidirectionalWithTimeout, T>::value>::type>
{
    /** The selected cluster connection type. */
    using type = ClusterConnectionBidirectionalWithTimeout;
};

} // namespace core
} // namespace middleware
