// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/InstancesDatabase.h"
#include "middleware/core/TransceiverBase.h"
#include "middleware/core/types.h"

#include <etl/span.h>

#include <cstdint>

namespace middleware::core
{

class EventSender;

/**
 * Base class for skeleton implementations.
 * This class provides the common functionality for all skeleton objects in the middleware.
 * Skeletons represent the server side of service communication, receiving method calls from
 * proxies and sending responses/events back. The SkeletonBase handles message processing, event
 * sending, and manages connections to multiple clusters.
 */
class SkeletonBase : public TransceiverBase
{
    friend class EventSender;

public:
    /**
     * Check if the skeleton is initialized.
     * Returns whether this skeleton has been properly initialized and is ready to
     * receive requests from proxies.
     *
     * \return true if initialized, false otherwise
     */
    bool isInitialized() const override;

    /**
     * Send a message through this skeleton.
     * Transmits the given message (typically a response or event) to proxies via the
     * cluster connections.
     *
     * \param msg reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    HRESULT sendMessage(Message& msg) const override;

    /** \see TransceiverBase::getSourceClusterId() */
    uint8_t getSourceClusterId() const final;

    /** Returns the span of cluster connections for this skeleton. */
    etl::span<IClusterConnection* const> const& getClusterConnections() const;

protected:
    virtual ~SkeletonBase();

    /** Unsubscribes this skeleton for \p serviceId. */
    void unsubscribe(uint16_t serviceId);

    /**
     * Check for cross-thread access violations.
     * Verifies that the current thread matches the initialization thread and logs an
     * error if a violation is detected.
     *
     * \param initId the ID of the thread that initialized this skeleton
     */
    void checkCrossThreadError(uint32_t initId) const;

    /**
     * Initialize the skeleton from the instances database.
     * Looks up the cluster connections for the specified instance ID in the given
     * database range and initializes the skeleton accordingly.
     *
     * \param instanceId the service instance ID to initialize for
     * \param dbRange span of instance database pointers to search
     * \return HRESULT indicating success or failure of the initialization
     */
    HRESULT initFromInstancesDatabase(
        uint16_t instanceId, etl::span<IInstanceDatabase const* const> const& dbRange);

    /** Span of cluster connections for this skeleton. */
    etl::span<IClusterConnection* const> _connections;

private:
    /** Returns INVALID_ADDRESS_ID (skeletons do not use address IDs). */
    uint8_t getAddressId() const final { return INVALID_ADDRESS_ID; }

    /** No-op: skeletons do not use address IDs. */
    void setAddressId(uint8_t const) final {}

    /** Returns the process/task ID; default returns INVALID_TASK_ID. */
    virtual uint32_t getProcessId() const { return INVALID_TASK_ID; }
};

} // namespace middleware::core
