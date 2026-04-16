// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/ITimeoutHandler.h"
#include "middleware/core/LoggerApi.h"
#include "middleware/core/Message.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware::core
{
/**
 * Base class for cluster connection implementations.
 * This class provides a base implementation of the IClusterConnection interface,
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
     * Main message processing function.
     * Processes incoming messages and releases message resources at the end of the
     * function. This is the entry point for handling messages received from other clusters.
     *
     * \param msg constant reference to the message to process
     */
    void processMessage(Message const& msg) const override;

    /** \see IClusterConnection::registeredTransceiversCount() */
    size_t registeredTransceiversCount(uint16_t const serviceId) const override
    {
        return _configuration.registeredTransceiversCount(serviceId);
    }

protected:
    /** Returns the configuration object. */
    IClusterConnectionConfigurationBase& getConfiguration() const { return _configuration; }

    /** Constructs from \p configuration. */
    explicit ClusterConnectionBase(IClusterConnectionConfigurationBase& configuration);

    /** \see IClusterConnection::getSourceClusterId() */
    uint8_t getSourceClusterId() const override { return _configuration.getSourceClusterId(); }

    /** \see IClusterConnection::getTargetClusterId() */
    uint8_t getTargetClusterId() const override { return _configuration.getTargetClusterId(); }

    /** \see IClusterConnection::sendMessage() */
    HRESULT sendMessage(Message const& msg) const override;

    /** \see IClusterConnection::dispatchMessage() */
    HRESULT dispatchMessage(Message const& msg) const override;

private:
    /**
     * Respond with an error message.
     * Sends an error response back to the sender when an error occurs during message
     * processing.
     *
     * \param error the error state to send
     * \param msg constant reference to the original message
     */
    void respondWithError(ErrorState const error, Message const& msg) const;

    /** Reference to the cluster connection configuration. */
    IClusterConnectionConfigurationBase& _configuration;
};

/**
 * Base class for cluster connections with timeout support.
 * This class extends ClusterConnectionBase with timeout management capabilities,
 * allowing transceivers to register for timeout notifications and handling periodic timeout
 * updates. It is useful for cluster connections that need to track and handle communication
 * timeouts.
 */
class ClusterConnectionTimeoutBase : public ClusterConnectionBase
{
public:
    /** \see IClusterConnection::registerTimeoutTransceiver() */
    void registerTimeoutTransceiver(ITimeoutHandler& transceiver) override;

    /** \see IClusterConnection::unregisterTimeoutTransceiver() */
    void unregisterTimeoutTransceiver(ITimeoutHandler& transceiver) override;

    /**
     * Update all registered timeout transceivers.
     * Processes timeout updates for all registered transceivers, checking for expired
     * timeouts and triggering appropriate notifications.
     */
    void updateTimeouts();

protected:
    /** Constructs from \p configuration. */
    explicit ClusterConnectionTimeoutBase(ITimeoutConfiguration& configuration);
};

} // namespace middleware::core
