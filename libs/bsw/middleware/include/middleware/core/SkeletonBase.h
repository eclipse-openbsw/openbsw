// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

#include <etl/span.h>

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/ITransceiver.h"
#include "middleware/core/InstancesDatabase.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

class EventSender;

/**
 * \brief Base class for skeleton implementations.
 * \details This class provides the common functionality for all skeleton objects in the middleware.
 * Skeletons represent the server side of service communication, receiving method calls from
 * proxies and sending responses/events back. The SkeletonBase handles message processing, event
 * sending, and manages connections to multiple clusters.
 */
class SkeletonBase : public ITransceiver
{
    friend class EventSender;

public:
    /**
     * \brief Check if the skeleton is initialized.
     * \details Returns whether this skeleton has been properly initialized and is ready to
     * receive requests from proxies.
     *
     * \return true if initialized, false otherwise
     */
    bool isInitialized() const override;

    /**
     * \brief Send a message through this skeleton.
     * \details Transmits the given message (typically a response or event) to proxies via the
     * cluster connections.
     *
     * \param msg reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    HRESULT sendMessage(Message& msg) const override;

    /**
     * \brief Get the source cluster ID.
     * \details Returns the identifier of the cluster where this skeleton resides.
     *
     * \return the source cluster ID
     */
    uint8_t getSourceClusterId() const final;

    /**
     * \brief Get the span of cluster connections.
     * \details Returns a reference to the span containing all cluster connections associated with
     * this skeleton, allowing the skeleton to communicate with proxies in multiple clusters.
     *
     * \return constant reference to the span of cluster connection pointers
     */
    etl::span<IClusterConnection* const> const& getClusterConnections() const;

protected:
    virtual ~SkeletonBase();

    /**
     * \brief Unsubscribe this skeleton from cluster connections.
     * \details Removes this skeleton from all cluster connections for the specified service ID,
     * stopping it from receiving further requests.
     *
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(uint16_t const serviceId);

    /**
     * \brief Check for cross-thread access violations.
     * \details Verifies that the current thread matches the initialization thread and logs an
     * error if a violation is detected.
     *
     * \param initId the ID of the thread that initialized this skeleton
     */
    void checkCrossThreadError(uint32_t const initId) const;

    /**
     * \brief Initialize the skeleton from the instances database.
     * \details Looks up the cluster connections for the specified instance ID in the given
     * database range and initializes the skeleton accordingly.
     *
     * \param instanceId the service instance ID to initialize for
     * \param dbRange span of instance database pointers to search
     * \return HRESULT indicating success or failure of the initialization
     */
    HRESULT initFromInstancesDatabase(
        uint16_t const instanceId, etl::span<IInstanceDatabase const* const> const& dbRange);

    etl::span<IClusterConnection* const>
        connections_; ///< Span of cluster connections for this skeleton

private:
    /**
     * \brief Get the address ID (not used for skeletons).
     * \details Skeletons do not use address IDs; always returns INVALID_ADDRESS_ID.
     *
     * \return INVALID_ADDRESS_ID
     */
    uint8_t getAddressId() const final { return INVALID_ADDRESS_ID; }

    /**
     * \brief Set the address ID (no-op for skeletons).
     * \details Skeletons do not use address IDs; this operation has no effect.
     *
     * \param addressId the address ID (ignored)
     */
    void setAddressId(uint8_t const) final {}

    /**
     * \brief Get the process/task ID (virtual for derived classes).
     * \details Returns the ID of the process/task that owns this skeleton, used for cross-thread
     * violation checks. Default implementation returns INVALID_TASK_ID.
     *
     * \return the process/task ID
     */
    virtual uint32_t getProcessId() const { return INVALID_TASK_ID; }
};

} // namespace core
} // namespace middleware
