// Copyright 2025 BMW AG

#pragma once

#include <etl/vector.h>

#include "middleware/core/ITimeout.h"
#include "middleware/core/ITransceiver.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
class ProxyBase;
class SkeletonBase;

namespace meta
{
struct TransceiverContainer;
}

/**
 * \brief Base interface for cluster connection configurations.
 * \details This interface defines the core configuration methods for cluster connections,
 * including cluster identification, message transmission, transceiver management, and message
 * dispatching. Implementations provide the specific behavior for different cluster connection
 * types.
 */
struct IClusterConnectionConfigurationBase
{
    /**
     * \brief Get the source cluster ID.
     * \return the source cluster ID
     */
    virtual uint8_t getSourceClusterId() const = 0;

    /**
     * \brief Get the target cluster ID.
     * \return the target cluster ID
     */
    virtual uint8_t getTargetClusterId() const = 0;

    /**
     * \brief Write a message to the cluster connection.
     * \param msg constant reference to the message to write
     * \return true if the write was successful, false otherwise
     */
    virtual bool write(Message const& msg) const = 0;

    /**
     * \brief Get the count of registered transceivers for a service.
     * \param serviceId the service ID to query
     * \return the number of registered transceivers
     */
    virtual size_t registeredTransceiversCount(uint16_t const serviceId) const = 0;

    /**
     * \brief Dispatch a message to its intended recipients.
     * \details Routes the message to the appropriate proxy or skeleton based on the message
     * header information.
     *
     * \param msg constant reference to the message to dispatch
     * \return HRESULT indicating success or failure of the dispatch operation
     */
    virtual HRESULT dispatchMessage(Message const& msg) const = 0;

    IClusterConnectionConfigurationBase(IClusterConnectionConfigurationBase const&) = delete;
    IClusterConnectionConfigurationBase& operator=(IClusterConnectionConfigurationBase const&)
        = delete;
    IClusterConnectionConfigurationBase(IClusterConnectionConfigurationBase&&)            = delete;
    IClusterConnectionConfigurationBase& operator=(IClusterConnectionConfigurationBase&&) = delete;

protected:
    virtual ~IClusterConnectionConfigurationBase() = default;
    IClusterConnectionConfigurationBase()          = default;

    /**
     * \brief Dispatch a message to proxy transceivers.
     * \details Routes the message to all proxy transceivers in the specified range.
     *
     * \param proxiesStart pointer to the start of the proxy transceiver container range
     * \param proxiesEnd pointer to the end of the proxy transceiver container range
     * \param msg constant reference to the message to dispatch
     * \return HRESULT indicating success or failure of the dispatch operation
     */
    static HRESULT dispatchMessageToProxy(
        meta::TransceiverContainer const* const proxiesStart,
        meta::TransceiverContainer const* const proxiesEnd,
        Message const& msg);

    /**
     * \brief Dispatch a message to skeleton transceivers.
     * \details Routes the message to all skeleton transceivers in the specified range.
     *
     * \param skeletonsStart pointer to the start of the skeleton transceiver container range
     * \param skeletonsEnd pointer to the end of the skeleton transceiver container range
     * \param msg constant reference to the message to dispatch
     * \return HRESULT indicating success or failure of the dispatch operation
     */
    static HRESULT dispatchMessageToSkeleton(
        meta::TransceiverContainer const* const skeletonsStart,
        meta::TransceiverContainer const* const skeletonsEnd,
        Message const& msg);

    /**
     * \brief Dispatch a message to both proxy and skeleton transceivers.
     * \details Routes the message to all transceivers (both proxies and skeletons) in the
     * specified ranges.
     *
     * \param proxiesStart pointer to the start of the proxy transceiver container range
     * \param proxiesEnd pointer to the end of the proxy transceiver container range
     * \param skeletonsStart pointer to the start of the skeleton transceiver container range
     * \param skeletonsEnd pointer to the end of the skeleton transceiver container range
     * \param msg constant reference to the message to dispatch
     * \return HRESULT indicating success or failure of the dispatch operation
     */
    static HRESULT dispatchMessage(
        meta::TransceiverContainer const* const proxiesStart,
        meta::TransceiverContainer const* const proxiesEnd,
        meta::TransceiverContainer const* const skeletonsStart,
        meta::TransceiverContainer const* const skeletonsEnd,
        Message const& msg);
};

/**
 * \brief Configuration interface for cluster connections with timeout support.
 * \details Extends the base configuration interface with timeout management capabilities,
 * allowing registration and management of timeout transceivers. This interface is used by
 * cluster connections that need to track and handle communication timeouts.
 */
struct ITimeoutConfiguration : public IClusterConnectionConfigurationBase
{
    /**
     * \brief Register a timeout transceiver.
     * \details Adds the given transceiver to the list of timeout transceivers that will be
     * notified of timeout events.
     *
     * \param transceiver reference to the ITimeout transceiver to register
     */
    virtual void registerTimeoutTransceiver(ITimeout& transceiver) = 0;

    /**
     * \brief Unregister a timeout transceiver.
     * \details Removes the given transceiver from the list of timeout transceivers.
     *
     * \param transceiver reference to the ITimeout transceiver to unregister
     */
    virtual void unregisterTimeoutTransceiver(ITimeout& transceiver) = 0;

    /**
     * \brief Update all registered timeout transceivers.
     * \details Processes timeout updates for all registered transceivers, checking for expired
     * timeouts and triggering appropriate notifications.
     */
    virtual void updateTimeouts() = 0;

    ITimeoutConfiguration(ITimeoutConfiguration const&)            = delete;
    ITimeoutConfiguration& operator=(ITimeoutConfiguration const&) = delete;
    ITimeoutConfiguration(ITimeoutConfiguration&&)                 = delete;
    ITimeoutConfiguration& operator=(ITimeoutConfiguration&&)      = delete;

protected:
    virtual ~ITimeoutConfiguration() = default;
    ITimeoutConfiguration()          = default;

    /**
     * \brief Static helper to register a timeout transceiver.
     * \details Adds the transceiver to the provided vector of timeout transceivers.
     *
     * \param transceiver reference to the ITimeout transceiver to register
     * \param timeoutTransceivers reference to the vector of timeout transceivers
     */
    static void registerTimeoutTransceiver(
        ITimeout& transceiver, ::etl::ivector<ITimeout*>& timeoutTransceivers);

    /**
     * \brief Static helper to unregister a timeout transceiver.
     * \details Removes the transceiver from the provided vector of timeout transceivers.
     *
     * \param transceiver reference to the ITimeout transceiver to unregister
     * \param timeoutTransceivers reference to the vector of timeout transceivers
     */
    static void unregisterTimeoutTransceiver(
        ITimeout& transceiver, ::etl::ivector<ITimeout*>& timeoutTransceivers);

    /**
     * \brief Static helper to update timeouts for all transceivers.
     * \details Processes timeout updates for all transceivers in the provided vector.
     *
     * \param timeoutTransceivers constant reference to the vector of timeout transceivers
     */
    static void updateTimeouts(::etl::ivector<ITimeout*> const& timeoutTransceivers);
};

/**
 * \brief Configuration interface for proxy-only cluster connections.
 * \details Extends the base configuration interface with proxy subscription management.
 * This interface is used by cluster connections that only support proxy communication.
 */
struct IClusterConnectionConfigurationProxyOnly : public IClusterConnectionConfigurationBase
{
    IClusterConnectionConfigurationProxyOnly(IClusterConnectionConfigurationProxyOnly const&)
        = delete;
    IClusterConnectionConfigurationProxyOnly&
    operator=(IClusterConnectionConfigurationProxyOnly const&)
        = delete;
    IClusterConnectionConfigurationProxyOnly(IClusterConnectionConfigurationProxyOnly&&) = delete;
    IClusterConnectionConfigurationProxyOnly& operator=(IClusterConnectionConfigurationProxyOnly&&)
        = delete;

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers the proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationProxyOnly() = default;
    IClusterConnectionConfigurationProxyOnly()          = default;
};

/**
 * \brief Configuration interface for skeleton-only cluster connections.
 * \details Extends the base configuration interface with skeleton subscription management.
 * This interface is used by cluster connections that only support skeleton communication.
 */
struct IClusterConnectionConfigurationSkeletonOnly : public IClusterConnectionConfigurationBase
{
    IClusterConnectionConfigurationSkeletonOnly(IClusterConnectionConfigurationSkeletonOnly const&)
        = delete;
    IClusterConnectionConfigurationSkeletonOnly&
    operator=(IClusterConnectionConfigurationSkeletonOnly const&)
        = delete;
    IClusterConnectionConfigurationSkeletonOnly(IClusterConnectionConfigurationSkeletonOnly&&)
        = delete;
    IClusterConnectionConfigurationSkeletonOnly&
    operator=(IClusterConnectionConfigurationSkeletonOnly&&)
        = delete;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers the skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationSkeletonOnly() = default;
    IClusterConnectionConfigurationSkeletonOnly()          = default;
};

/**
 * \brief Configuration interface for bidirectional cluster connections.
 * \details Extends the base configuration interface with both proxy and skeleton subscription
 * management. This interface is used by cluster connections that support full bidirectional
 * communication.
 */
struct IClusterConnectionConfigurationBidirectional : public IClusterConnectionConfigurationBase
{
    IClusterConnectionConfigurationBidirectional(
        IClusterConnectionConfigurationBidirectional const&)
        = delete;
    IClusterConnectionConfigurationBidirectional&
    operator=(IClusterConnectionConfigurationBidirectional const&)
        = delete;
    IClusterConnectionConfigurationBidirectional(IClusterConnectionConfigurationBidirectional&&)
        = delete;
    IClusterConnectionConfigurationBidirectional&
    operator=(IClusterConnectionConfigurationBidirectional&&)
        = delete;

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers the proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers the skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationBidirectional() = default;
    IClusterConnectionConfigurationBidirectional()          = default;
};

/**
 * \brief Configuration interface for proxy-only cluster connections with timeout support.
 * \details Extends the timeout configuration interface with proxy subscription management.
 * This interface is used by cluster connections that support proxy communication with timeout
 * tracking.
 */
struct IClusterConnectionConfigurationProxyOnlyWithTimeout : public ITimeoutConfiguration
{
    IClusterConnectionConfigurationProxyOnlyWithTimeout(
        IClusterConnectionConfigurationProxyOnlyWithTimeout const&)
        = delete;
    IClusterConnectionConfigurationProxyOnlyWithTimeout&
    operator=(IClusterConnectionConfigurationProxyOnlyWithTimeout const&)
        = delete;
    IClusterConnectionConfigurationProxyOnlyWithTimeout(
        IClusterConnectionConfigurationProxyOnlyWithTimeout&&)
        = delete;
    IClusterConnectionConfigurationProxyOnlyWithTimeout&
    operator=(IClusterConnectionConfigurationProxyOnlyWithTimeout&&)
        = delete;

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers the proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationProxyOnlyWithTimeout() = default;
    IClusterConnectionConfigurationProxyOnlyWithTimeout()          = default;
};

/**
 * \brief Configuration interface for bidirectional cluster connections with timeout support.
 * \details Extends the timeout configuration interface with both proxy and skeleton subscription
 * management. This interface is used by cluster connections that support full bidirectional
 * communication with timeout tracking.
 */
struct IClusterConnectionConfigurationBidirectionalWithTimeout : public ITimeoutConfiguration
{
    IClusterConnectionConfigurationBidirectionalWithTimeout(
        IClusterConnectionConfigurationBidirectionalWithTimeout const&)
        = delete;
    IClusterConnectionConfigurationBidirectionalWithTimeout&
    operator=(IClusterConnectionConfigurationBidirectionalWithTimeout const&)
        = delete;
    IClusterConnectionConfigurationBidirectionalWithTimeout(
        IClusterConnectionConfigurationBidirectionalWithTimeout&&)
        = delete;
    IClusterConnectionConfigurationBidirectionalWithTimeout&
    operator=(IClusterConnectionConfigurationBidirectionalWithTimeout&&)
        = delete;

    /**
     * \brief Subscribe a proxy to the cluster connection.
     * \details Registers the proxy with the specified service instance ID to receive messages.
     *
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \details Removes the proxy with the specified service ID from the cluster connection.
     *
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers the skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    IClusterConnectionConfigurationBidirectionalWithTimeout()          = default;
    virtual ~IClusterConnectionConfigurationBidirectionalWithTimeout() = default;
};

/**
 * \brief Configuration interface for skeleton-only cluster connections with timeout support.
 * \details Extends the timeout configuration interface with skeleton subscription management.
 * This interface is used by cluster connections that support skeleton communication with timeout
 * tracking.
 */
struct IClusterConnectionConfigurationSkeletonOnlyWithTimeout : public ITimeoutConfiguration
{
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout(
        IClusterConnectionConfigurationSkeletonOnlyWithTimeout const&)
        = delete;
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout&
    operator=(IClusterConnectionConfigurationSkeletonOnlyWithTimeout const&)
        = delete;
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout(
        IClusterConnectionConfigurationSkeletonOnlyWithTimeout&&)
        = delete;
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout&
    operator=(IClusterConnectionConfigurationSkeletonOnlyWithTimeout&&)
        = delete;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \details Registers the skeleton with the specified service instance ID to receive messages.
     *
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \details Removes the skeleton with the specified service ID from the cluster connection.
     *
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationSkeletonOnlyWithTimeout() = default;
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout()          = default;
};

} // namespace core
} // namespace middleware
