// Copyright 2025 BMW AG

#pragma once

#include "IClusterConnection.h"
#include "InstancesDatabase.h"
#include "TransceiverBase.h"

#include <etl/span.h>

#include <cstdint>

namespace middleware
{
namespace core
{

/**
 * Base class for proxy implementations.
 * This class provides the common functionality for all proxy objects in the middleware.
 * Proxies represent the client side of service communication, allowing applications to call
 * methods on remote service instances (skeletons). The ProxyBase handles message generation,
 * sending, and cluster connection management.
 */
class ProxyBase : public TransceiverBase
{
public:
    /**
     * Set the address ID for this proxy.
     * Updates the unique address identifier for this proxy instance, used for routing
     * response messages back to this specific proxy.
     *
     * \param addressId the new address ID to set
     */
    void setAddressId(uint8_t addressId) final;

    /** \see TransceiverBase::getAddressId() */
    uint8_t getAddressId() const final;

    /**
     * Check if the proxy is initialized.
     * Returns whether this proxy has been properly initialized and is ready to
     * communicate with skeletons.
     *
     * \return true if initialized, false otherwise
     */
    bool isInitialized() const override;

    /**
     * Generate a message header for a request.
     * Creates a message header with the proxy's service information and the specified
     * member ID and request ID. This is typically used when preparing to send a method call or
     * request to a skeleton.
     *
     * \param memberId the member (method/event) ID within the service
     * \param requestId the request ID for the message (defaults to INVALID_REQUEST_ID)
     * \return the generated message with header populated
     */
    [[nodiscard]] Message
    generateMessageHeader(uint16_t memberId, uint16_t requestId = INVALID_REQUEST_ID) const;

    /**
     * Send a message through this proxy.
     * Transmits the given message to the skeleton via the cluster connection.
     *
     * \param msg reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    [[nodiscard]] HRESULT sendMessage(Message& msg) const override;

protected:
    constexpr ProxyBase() : TransceiverBase(), _addressId(INVALID_ADDRESS_ID) {}

    virtual ~ProxyBase() = default;

    /** \see TransceiverBase::getSourceClusterId() */
    uint8_t getSourceClusterId() const final;

    /** Unsubscribes this proxy for \p serviceId. */
    void unsubscribe(uint16_t serviceId);

    /**
     * Check for cross-thread access violations.
     * Verifies that the current thread matches the initialization thread and logs an
     * error if a violation is detected.
     *
     * \param initId the ID of the thread that initialized this proxy
     */
    void checkCrossThreadError(uint32_t initId) const;

    /**
     * Initialize the proxy from the instances database.
     * Looks up the cluster connection for the specified instance ID and source cluster
     * in the given database range and initializes the proxy accordingly.
     *
     * \param instanceId the service instance ID to initialize for
     * \param sourceCluster the source cluster ID
     * \param dbRange span of instance database pointers to search
     * \return HRESULT indicating success or failure of the initialization
     */
    HRESULT initFromInstancesDatabase(
        uint16_t instanceId,
        uint8_t sourceCluster,
        etl::span<IInstanceDatabase const* const> const& dbRange);

    /** Pointer to the cluster connection for this proxy. */
    IClusterConnection* _connection{nullptr};

private:
    /** The unique address ID for this proxy instance. */
    uint8_t _addressId;
};

} // namespace core
} // namespace middleware
