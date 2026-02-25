// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/ITimeout.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
/**
 * \brief Base class for cluster connection implementations.
 * \details This class provides a base implementation of the IClusterConnection interface,
 * handling common functionality for cluster connections such as message processing,
 * transceiver management, and message dispatching. It maintains a reference to the
 * cluster connection configuration and implements core message routing logic.
 */
class ClusterConnectionBase : public IClusterConnection
{
public:
    ClusterConnectionBase(ClusterConnectionBase const&)            = delete;
    ClusterConnectionBase& operator=(ClusterConnectionBase const&) = delete;
    ClusterConnectionBase(ClusterConnectionBase&&)                 = delete;
    ClusterConnectionBase& operator=(ClusterConnectionBase&&)      = delete;

    /**
     * \brief Main message processing function.
     * \details Processes incoming messages and releases message resources at the end of the
     * function. This is the entry point for handling messages received from other clusters.
     *
     * \param msg constant reference to the message to process
     */
    void processMessage(Message const& msg) const override;

    /**
     * \brief Get the count of registered transceivers for a service.
     * \param serviceId the service ID to query
     * \return the number of registered transceivers
     */
    size_t registeredTransceiversCount(uint16_t const serviceId) const override
    {
        return fConfiguration.registeredTransceiversCount(serviceId);
    }

protected:
    /**
     * \brief Get the configuration object.
     * \return reference to the IClusterConnectionConfigurationBase configuration
     */
    IClusterConnectionConfigurationBase& getConfiguration() const { return fConfiguration; }

    /**
     * \brief Constructor for ClusterConnectionBase.
     * \param configuration reference to the cluster connection configuration
     */
    explicit ClusterConnectionBase(IClusterConnectionConfigurationBase& configuration);

    /**
     * \brief Get the source cluster ID.
     * \return the source cluster ID
     */
    uint8_t getSourceClusterId() const override { return fConfiguration.getSourceClusterId(); }

    /**
     * \brief Get the target cluster ID.
     * \return the target cluster ID
     */
    uint8_t getTargetClusterId() const override { return fConfiguration.getTargetClusterId(); }

    /**
     * \brief Send a message through the cluster connection.
     * \details Transmits the given message to the target cluster specified in the message header.
     *
     * \param msg constant reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    HRESULT sendMessage(Message const& msg) const override;

    /**
     * \brief Dispatch a message to its intended recipients.
     * \details Routes the message to the appropriate proxy or skeleton based on the message header
     * information.
     *
     * \param msg constant reference to the message to dispatch
     * \return HRESULT indicating success or failure of the dispatch operation
     */
    HRESULT dispatchMessage(Message const& msg) const override;

private:
    /**
     * \brief Respond with an error message.
     * \details Sends an error response back to the sender when an error occurs during message
     * processing.
     *
     * \param error the error state to send
     * \param msg constant reference to the original message
     */
    void respondWithError(ErrorState const error, Message const& msg) const;

    IClusterConnectionConfigurationBase&
        fConfiguration; ///< Reference to the cluster connection configuration
};

/**
 * \brief Base class for cluster connections with timeout support.
 * \details This class extends ClusterConnectionBase with timeout management capabilities,
 * allowing transceivers to register for timeout notifications and handling periodic timeout
 * updates. It is useful for cluster connections that need to track and handle communication
 * timeouts.
 */
class ClusterConnectionTimeoutBase : public ClusterConnectionBase
{
public:
    /**
     * \brief Register a timeout transceiver.
     * \param transceiver reference to the ITimeout transceiver to register
     */
    void registerTimeoutTransceiver(ITimeout& transceiver) override;

    /**
     * \brief Unregister a timeout transceiver.
     * \param transceiver reference to the ITimeout transceiver to unregister
     */
    void unregisterTimeoutTransceiver(ITimeout& transceiver) override;

    /**
     * \brief Update all registered timeout transceivers.
     * \details Processes timeout updates for all registered transceivers, checking for expired
     * timeouts and triggering appropriate notifications.
     */
    void updateTimeouts();

protected:
    /**
     * \brief Constructor for ClusterConnectionTimeoutBase.
     * \param configuration reference to the timeout configuration
     */
    explicit ClusterConnectionTimeoutBase(ITimeoutConfiguration& configuration);
};

} // namespace core
} // namespace middleware
