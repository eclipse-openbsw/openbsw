// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/ITimeoutHandler.h"
#include "middleware/core/Message.h"
#include "middleware/core/TransceiverBase.h"
#include "middleware/core/types.h"

namespace middleware::core
{

class SkeletonBase;
class ProxyBase;

/**
 * Interface for timeout cluster connection.
 * This interface provides basic timeout management functionality for cluster connections.
 * It allows transceivers to register and unregister for timeout notifications.
 */
class ITimeoutClusterConnection
{
public:
    /** Registers \p timeout as a timeout transceiver (no-op by default). */
    virtual void registerTimeoutTransceiver(ITimeoutHandler&) {}

    /** Unregisters \p timeout from timeout transceivers (no-op by default). */
    virtual void unregisterTimeoutTransceiver(ITimeoutHandler&) {}
};

/**
 * Interface for cluster connection management.
 * This interface provides the core functionality for managing connections between clusters
 * in the middleware. It handles subscription management for proxies and skeletons, message routing,
 * and cluster identification. A cluster connection is responsible for sending and receiving
 * messages between different clusters, maintaining the registry of transceivers and dispatching
 * messages to the appropriate recipients.
 */
class IClusterConnection : public ITimeoutClusterConnection
{
public:
    /** Returns the source cluster ID. */
    virtual uint8_t getSourceClusterId() const = 0;

    /** Returns the target cluster ID. */
    virtual uint8_t getTargetClusterId() const = 0;

    /** Subscribes \p proxy for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /** Subscribes \p skeleton for \p serviceInstanceId, returns HRESULT. */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /** Unsubscribes \p proxy for \p serviceId. */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

    /** Unsubscribes \p skeleton for \p serviceId. */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

    /**
     * Send a message through the cluster connection.
     * Transmits the given message to the target cluster specified in the message header.
     *
     * \param msg constant reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    virtual HRESULT sendMessage(Message const& msg) const = 0;

    /**
     * Process an incoming message.
     * Processes a message received from another cluster, performing any necessary
     * validation and routing operations.
     *
     * \param msg constant reference to the message to process
     */
    virtual void processMessage(Message const& msg) const = 0;

    /** Returns count of registered transceivers for a \p serviceId. */
    virtual size_t registeredTransceiversCount(uint16_t const serviceId) const = 0;

    /**
     * Dispatch a message to its intended recipients.
     * Routes the message to the appropriate proxy or skeleton based on the message header
     * information.
     *
     * \param msg constant reference to the message to dispatch
     * \return HRESULT indicating success or failure of the dispatch operation
     */
    virtual HRESULT dispatchMessage(Message const& msg) const = 0;

    IClusterConnection(IClusterConnection const&)            = delete;
    IClusterConnection& operator=(IClusterConnection const&) = delete;
    IClusterConnection(IClusterConnection&&)                 = delete;
    IClusterConnection& operator=(IClusterConnection&&)      = delete;

protected:
    IClusterConnection() = default;
};

} // namespace middleware::core
