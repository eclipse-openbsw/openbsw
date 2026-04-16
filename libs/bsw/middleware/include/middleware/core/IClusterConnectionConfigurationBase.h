// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/ITimeoutHandler.h"
#include "middleware/core/Message.h"
#include "middleware/core/TransceiverBase.h"
#include "middleware/core/types.h"

#include <etl/vector.h>

namespace middleware::core
{
class ProxyBase;
class SkeletonBase;

namespace meta
{
struct TransceiverContainer;
}

/**
 * Base interface for cluster connection configurations.
 * This interface defines the core configuration methods for cluster connections,
 * including cluster identification, message transmission, transceiver management, and message
 * dispatching. Implementations provide the specific behavior for different cluster connection
 * types.
 */
struct IClusterConnectionConfigurationBase
{
    /** Returns the source cluster ID. */
    virtual uint8_t getSourceClusterId() const = 0;

    /** Returns the target cluster ID. */
    virtual uint8_t getTargetClusterId() const = 0;

    /** Writes \p msg to the cluster connection, returns true on success. */
    virtual bool write(Message const& msg) const = 0;

    /** Returns count of registered transceivers for \p serviceId. */
    virtual size_t registeredTransceiversCount(uint16_t const serviceId) const = 0;

    /**
     * Dispatch a message to its intended recipients.
     * Routes the message to the appropriate proxy or skeleton based on the message
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
     * Dispatch a message to proxy transceivers.
     * Routes the message to all proxy transceivers in the specified range.
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
     * Dispatch a message to skeleton transceivers.
     * Routes the message to all skeleton transceivers in the specified range.
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
     * Dispatch a message to both proxy and skeleton transceivers.
     * Routes the message to all transceivers (both proxies and skeletons) in the
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
 * Configuration interface for cluster connections with timeout support.
 * Extends the base configuration interface with timeout management capabilities,
 * allowing registration and management of timeout transceivers. This interface is used by
 * cluster connections that need to track and handle communication timeouts.
 */
struct ITimeoutConfiguration : public IClusterConnectionConfigurationBase
{
    /** Registers \p transceiver for timeout events. */
    virtual void registerTimeoutTransceiver(ITimeoutHandler& transceiver) = 0;

    /** Unregisters \p transceiver from timeout events. */
    virtual void unregisterTimeoutTransceiver(ITimeoutHandler& transceiver) = 0;

    /**
     * Update all registered timeout transceivers.
     * Processes timeout updates for all registered transceivers, checking for expired
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

    /** Adds \p transceiver to \p timeoutTransceivers. */
    static void registerTimeoutTransceiver(
        ITimeoutHandler& transceiver, ::etl::ivector<ITimeoutHandler*>& timeoutTransceivers);

    /** Removes \p transceiver from \p timeoutTransceivers. */
    static void unregisterTimeoutTransceiver(
        ITimeoutHandler& transceiver, ::etl::ivector<ITimeoutHandler*>& timeoutTransceivers);

    /** Updates all timeouts in \p timeoutTransceivers. */
    static void updateTimeouts(::etl::ivector<ITimeoutHandler*> const& timeoutTransceivers);
};

/**
 * Configuration interface for proxy-only cluster connections.
 * Extends the base configuration interface with proxy subscription management.
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

    /** Subscribes \p proxy for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p proxy for \p serviceId. */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationProxyOnly() = default;
    IClusterConnectionConfigurationProxyOnly()          = default;
};

/**
 * Configuration interface for skeleton-only cluster connections.
 * Extends the base configuration interface with skeleton subscription management.
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

    /** Subscribes \p skeleton for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p skeleton for \p serviceId. */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationSkeletonOnly() = default;
    IClusterConnectionConfigurationSkeletonOnly()          = default;
};

/**
 * Configuration interface for bidirectional cluster connections.
 * Extends the base configuration interface with both proxy and skeleton subscription
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

    /** Subscribes \p proxy for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p proxy for \p serviceId. */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

    /** Subscribes \p skeleton for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p skeleton for \p serviceId. */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationBidirectional() = default;
    IClusterConnectionConfigurationBidirectional()          = default;
};

/**
 * Configuration interface for proxy-only cluster connections with timeout support.
 * Extends the timeout configuration interface with proxy subscription management.
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

    /** Subscribes \p proxy for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p proxy for \p serviceId. */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationProxyOnlyWithTimeout() = default;
    IClusterConnectionConfigurationProxyOnlyWithTimeout()          = default;
};

/**
 * Configuration interface for bidirectional cluster connections with timeout support.
 * Extends the timeout configuration interface with both proxy and skeleton subscription
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

    /** Subscribes \p proxy for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p proxy for \p serviceId. */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

    /** Subscribes \p skeleton for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p skeleton for \p serviceId. */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    IClusterConnectionConfigurationBidirectionalWithTimeout()          = default;
    virtual ~IClusterConnectionConfigurationBidirectionalWithTimeout() = default;
};

/**
 * Configuration interface for skeleton-only cluster connections with timeout support.
 * Extends the timeout configuration interface with skeleton subscription management.
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

    /** Subscribes \p skeleton for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p skeleton for \p serviceId. */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

protected:
    virtual ~IClusterConnectionConfigurationSkeletonOnlyWithTimeout() = default;
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout()          = default;
};

} // namespace middleware::core
