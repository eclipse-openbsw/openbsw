// Copyright 2025 BMW AG

#pragma once

#include <etl/type_traits.h>

#include "middleware/core/ClusterConnectionBase.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/Message.h"

namespace middleware
{
namespace core
{
class ProxyBase;
class SkeletonBase;

/**
 * \brief Cluster connection for proxy-only communication without timeout support.
 * \details This class provides a cluster connection that only supports proxy subscriptions,
 * without timeout management. Skeleton subscriptions are not implemented and will return
 * NotImplemented.
 */
class ClusterConnectionNoTimeoutProxyOnly final : public ClusterConnectionBase
{
    using Base = ClusterConnectionBase;

public:
    /**
     * \brief Constructor for ClusterConnectionNoTimeoutProxyOnly.
     * \details Initializes the proxy-only cluster connection with the provided configuration.
     *
     * \param configuration reference to the proxy-only cluster connection configuration
     */
    explicit ClusterConnectionNoTimeoutProxyOnly(
        IClusterConnectionConfigurationProxyOnly& configuration);

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers a proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /**
     * \brief Subscribe a skeleton (not implemented).
     * \details This operation is not supported for proxy-only connections.
     *
     * \return HRESULT::NotImplemented
     */
    HRESULT subscribe(SkeletonBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /**
     * \brief Unsubscribe a skeleton (no-op).
     * \details This operation has no effect for proxy-only connections.
     */
    void unsubscribe(SkeletonBase&, uint16_t const) final {}
};

/**
 * \brief Cluster connection for skeleton-only communication without timeout support.
 * \details This class provides a cluster connection that only supports skeleton subscriptions,
 * without timeout management. Proxy subscriptions are not implemented and will return
 * NotImplemented.
 */
class ClusterConnectionNoTimeoutSkeletonOnly final : public ClusterConnectionBase
{
    using Base = ClusterConnectionBase;

public:
    /**
     * \brief Constructor for ClusterConnectionNoTimeoutSkeletonOnly.
     * \details Initializes the skeleton-only cluster connection with the provided configuration.
     *
     * \param configuration reference to the skeleton-only cluster connection configuration
     */
    explicit ClusterConnectionNoTimeoutSkeletonOnly(
        IClusterConnectionConfigurationSkeletonOnly& configuration);

    /**
     * \brief Subscribe a proxy (not implemented).
     * \details This operation is not supported for skeleton-only connections.
     *
     * \return HRESULT::NotImplemented
     */
    HRESULT subscribe(ProxyBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers a skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) final;

    /**
     * \brief Unsubscribe a proxy (no-op).
     * \details This operation has no effect for skeleton-only connections.
     */
    void unsubscribe(ProxyBase&, uint16_t const) final {}

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) final;
};

/**
 * \brief Cluster connection for bidirectional communication without timeout support.
 * \details This class provides a cluster connection that supports both proxy and skeleton
 * subscriptions, without timeout management. It enables full bidirectional communication
 * between clusters.
 */
class ClusterConnectionNoTimeoutBidirectional final : public ClusterConnectionBase
{
    using Base = ClusterConnectionBase;

public:
    /**
     * \brief Constructor for ClusterConnectionNoTimeoutBidirectional.
     * \details Initializes the bidirectional cluster connection with the provided configuration.
     *
     * \param configuration reference to the bidirectional cluster connection configuration
     */
    explicit ClusterConnectionNoTimeoutBidirectional(
        IClusterConnectionConfigurationBidirectional& configuration);

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers a proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers a skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) final;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) final;
};

/**
 * \brief Cluster connection for bidirectional communication with timeout support.
 * \details This class provides a cluster connection that supports both proxy and skeleton
 * subscriptions, with timeout management capabilities. It enables full bidirectional
 * communication between clusters with timeout tracking.
 */
class ClusterConnectionBidirectionalWithTimeout final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /**
     * \brief Constructor for ClusterConnectionBidirectionalWithTimeout.
     * \details Initializes the bidirectional cluster connection with timeout support.
     *
     * \param configuration reference to the bidirectional timeout configuration
     */
    explicit ClusterConnectionBidirectionalWithTimeout(
        IClusterConnectionConfigurationBidirectionalWithTimeout& configuration);

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers a proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers a skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) final;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) final;
};

/**
 * \brief Cluster connection for proxy-only communication with timeout support.
 * \details This class provides a cluster connection that only supports proxy subscriptions,
 * with timeout management capabilities. Skeleton subscriptions are not implemented.
 */
class ClusterConnectionProxyOnlyWithTimeout final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /**
     * \brief Constructor for ClusterConnectionProxyOnlyWithTimeout.
     * \details Initializes the proxy-only cluster connection with timeout support.
     *
     * \param configuration reference to the proxy-only timeout configuration
     */
    explicit ClusterConnectionProxyOnlyWithTimeout(
        IClusterConnectionConfigurationProxyOnlyWithTimeout& configuration);

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers a proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) final;

    /**
     * \brief Subscribe a skeleton (not implemented).
     * \details This operation is not supported for proxy-only connections.
     *
     * \return HRESULT::NotImplemented
     */
    HRESULT subscribe(SkeletonBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) final;

    /**
     * \brief Unsubscribe a skeleton (no-op).
     * \details This operation has no effect for proxy-only connections.
     */
    void unsubscribe(SkeletonBase&, uint16_t const) final {}
};

/**
 * \brief Cluster connection for skeleton-only communication with timeout support.
 * \details This class provides a cluster connection that only supports skeleton subscriptions,
 * with timeout management capabilities. Proxy subscriptions are not implemented.
 */
class ClusterConnectionSkeletonOnlyWithTimeout final : public ClusterConnectionTimeoutBase
{
    using Base = ClusterConnectionTimeoutBase;

public:
    /**
     * \brief Constructor for ClusterConnectionSkeletonOnlyWithTimeout.
     * \details Initializes the skeleton-only cluster connection with timeout support.
     *
     * \param configuration reference to the skeleton-only timeout configuration
     */
    explicit ClusterConnectionSkeletonOnlyWithTimeout(
        IClusterConnectionConfigurationSkeletonOnlyWithTimeout& configuration);

    /**
     * \brief Subscribe a proxy (not implemented).
     * \details This operation is not supported for skeleton-only connections.
     *
     * \return HRESULT::NotImplemented
     */
    HRESULT subscribe(ProxyBase&, uint16_t const) final { return HRESULT::NotImplemented; }

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers a skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    HRESULT subscribe(SkeletonBase&, uint16_t const) final;

    /**
     * \brief Unsubscribe a proxy (no-op).
     * \details This operation has no effect for skeleton-only connections.
     */
    void
    unsubscribe([[maybe_unused]] ProxyBase& proxy, [[maybe_unused]] uint16_t const serviceId) final
    {}

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(SkeletonBase&, [[maybe_unused]] uint16_t const) final;
};

/**
 * \brief Type selector for cluster connection implementations.
 * \details This template struct selects the appropriate cluster connection type based on the
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
 * \brief Type selector specialization for proxy-only configurations.
 * \tparam T the proxy-only configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationProxyOnly, T>::value>::type>
{
    using type = ClusterConnectionNoTimeoutProxyOnly; ///< The selected cluster connection type
};

/**
 * \brief Type selector specialization for skeleton-only configurations.
 * \tparam T the skeleton-only configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationSkeletonOnly, T>::value>::type>
{
    using type = ClusterConnectionNoTimeoutSkeletonOnly; ///< The selected cluster connection type
};

/**
 * \brief Type selector specialization for bidirectional configurations.
 * \tparam T the bidirectional configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationBidirectional, T>::value>::type>
{
    using type = ClusterConnectionNoTimeoutBidirectional; ///< The selected cluster connection type
};

/**
 * \brief Type selector specialization for proxy-only configurations with timeout.
 * \tparam T the proxy-only with timeout configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationProxyOnlyWithTimeout, T>::value>::type>
{
    using type = ClusterConnectionProxyOnlyWithTimeout; ///< The selected cluster connection type
};

/**
 * \brief Type selector specialization for skeleton-only configurations with timeout.
 * \tparam T the skeleton-only with timeout configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationSkeletonOnlyWithTimeout, T>::value>::type>
{
    using type = ClusterConnectionSkeletonOnlyWithTimeout; ///< The selected cluster connection type
};

/**
 * \brief Type selector specialization for bidirectional configurations with timeout.
 * \tparam T the bidirectional with timeout configuration type
 */
template<typename T>
struct ClusterConnectionTypeSelector<
    T,
    typename etl::enable_if<
        etl::is_base_of<IClusterConnectionConfigurationBidirectionalWithTimeout, T>::value>::type>
{
    using type
        = ClusterConnectionBidirectionalWithTimeout; ///< The selected cluster connection type
};

} // namespace core
} // namespace middleware
