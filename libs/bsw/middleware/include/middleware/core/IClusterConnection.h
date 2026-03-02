// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/ITimeout.h"
#include "middleware/core/ITransceiver.h"
#include "middleware/core/Message.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

class SkeletonBase;
class ProxyBase;

/**
 * \brief Interface for timeout cluster connection.
 * \details This interface provides basic timeout management functionality for cluster connections.
 * It allows transceivers to register and unregister for timeout notifications.
 */
class ITimeoutClusterConnection
{
public:
    /**
     * \brief Register a timeout transceiver.
     *
     * \param timeout reference to the ITimeout transceiver to register
     */
    virtual void registerTimeoutTransceiver(ITimeout&) {}

    /**
     * \brief Unregister a timeout transceiver.
     *
     * \param timeout reference to the ITimeout transceiver to unregister
     */
    virtual void unregisterTimeoutTransceiver(ITimeout&) {}
};

/**
 * \brief Interface for cluster connection management.
 * \details This interface provides the core functionality for managing connections between clusters
 * in the middleware. It handles subscription management for proxies and skeletons, message routing,
 * and cluster identification. A cluster connection is responsible for sending and receiving
 * messages between different clusters, maintaining the registry of transceivers and dispatching
 * messages to the appropriate recipients.
 */
class IClusterConnection : public ITimeoutClusterConnection
{
public:
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
     * \brief Subscribe a proxy to the cluster connection.
     * \param proxy reference to the proxy to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Subscribe a skeleton to the cluster connection.
     * \param skeleton reference to the skeleton to subscribe
     * \param serviceInstanceId the service instance ID for the subscription
     * \return HRESULT indicating success or failure of the subscription
     */
    virtual HRESULT subscribe(SkeletonBase& skeleton, uint16_t const serviceInstanceId) = 0;

    /**
     * \brief Unsubscribe a proxy from the cluster connection.
     * \param proxy reference to the proxy to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(ProxyBase& proxy, uint16_t const serviceId) = 0;

    /**
     * \brief Unsubscribe a skeleton from the cluster connection.
     * \param skeleton reference to the skeleton to unsubscribe
     * \param serviceId the service ID for the unsubscription
     */
    virtual void unsubscribe(SkeletonBase& skeleton, uint16_t const serviceId) = 0;

    /**
     * \brief Send a message through the cluster connection.
     * \details Transmits the given message to the target cluster specified in the message header.
     *
     * \param msg constant reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    virtual HRESULT sendMessage(Message const& msg) const = 0;

    /**
     * \brief Process an incoming message.
     * \details Processes a message received from another cluster, performing any necessary
     * validation and routing operations.
     *
     * \param msg constant reference to the message to process
     */
    virtual void processMessage(Message const& msg) const = 0;

    /**
     * \brief Get the count of registered transceivers for a service.
     * \param serviceId the service ID to query
     * \return the number of registered transceivers
     */
    virtual size_t registeredTransceiversCount(uint16_t const serviceId) const = 0;

    /**
     * \brief Dispatch a message to its intended recipients.
     * \details Routes the message to the appropriate proxy or skeleton based on the message header
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

} // namespace core
} // namespace middleware
