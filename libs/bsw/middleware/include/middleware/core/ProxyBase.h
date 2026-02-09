// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

#include <etl/span.h>

#include "IClusterConnection.h"
#include "ITransceiver.h"
#include "InstancesDatabase.h"

namespace middleware
{
namespace core
{

/**
 * \brief Base class for proxy implementations.
 * \details This class provides the common functionality for all proxy objects in the middleware.
 * Proxies represent the client side of service communication, allowing applications to call
 * methods on remote service instances (skeletons). The ProxyBase handles message generation,
 * sending, and cluster connection management.
 */
class ProxyBase : public ITransceiver
{
public:
    /**
     * \brief Set the address ID for this proxy.
     * \details Updates the unique address identifier for this proxy instance, used for routing
     * response messages back to this specific proxy.
     *
     * \param addressId the new address ID to set
     */
    void setAddressId(uint8_t const addressId) final;

    /**
     * \brief Get the address ID of this proxy.
     * \details Returns the unique address identifier for this proxy instance.
     *
     * \return the address ID
     */
    uint8_t getAddressId() const final;

    /**
     * \brief Check if the proxy is initialized.
     * \details Returns whether this proxy has been properly initialized and is ready to
     * communicate with skeletons.
     *
     * \return true if initialized, false otherwise
     */
    bool isInitialized() const override;

    /**
     * \brief Generate a message header for a request.
     * \details Creates a message header with the proxy's service information and the specified
     * member ID and request ID. This is typically used when preparing to send a method call or
     * request to a skeleton.
     *
     * \param memberId the member (method/event) ID within the service
     * \param requestId the request ID for the message (defaults to INVALID_REQUEST_ID)
     * \return the generated message with header populated
     */
    [[nodiscard]] Message generateMessageHeader(
        uint16_t const memberId, uint16_t const requestId = INVALID_REQUEST_ID) const;

    /**
     * \brief Send a message through this proxy.
     * \details Transmits the given message to the skeleton via the cluster connection.
     *
     * \param msg reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    [[nodiscard]] HRESULT sendMessage(Message& msg) const override;

protected:
    constexpr ProxyBase() : ITransceiver(), addressId_(INVALID_ADDRESS_ID) {}

    virtual ~ProxyBase() = default;

    /**
     * \brief Get the source cluster ID from the connection.
     * \details Returns the identifier of the cluster where this proxy resides.
     *
     * \return the source cluster ID
     */
    uint8_t getSourceClusterId() const final;

    /**
     * \brief Unsubscribe this proxy from the cluster connection.
     * \details Removes this proxy from the cluster connection for the specified service ID,
     * stopping it from receiving further messages.
     *
     * \param serviceId the service ID for the unsubscription
     */
    void unsubscribe(uint16_t const serviceId);

    /**
     * \brief Check for cross-thread access violations.
     * \details Verifies that the current thread matches the initialization thread and logs an
     * error if a violation is detected.
     *
     * \param initId the ID of the thread that initialized this proxy
     */
    void checkCrossThreadError(uint32_t const initId) const;

    /**
     * \brief Initialize the proxy from the instances database.
     * \details Looks up the cluster connection for the specified instance ID and source cluster
     * in the given database range and initializes the proxy accordingly.
     *
     * \param instanceId the service instance ID to initialize for
     * \param sourceCluster the source cluster ID
     * \param dbRange span of instance database pointers to search
     * \return HRESULT indicating success or failure of the initialization
     */
    HRESULT initFromInstancesDatabase(
        uint16_t const instanceId,
        uint8_t const sourceCluster,
        etl::span<IInstanceDatabase const* const> const& dbRange);

    IClusterConnection* fConnection{nullptr}; ///< Pointer to the cluster connection for this proxy

private:
    uint8_t addressId_; ///< The unique address ID for this proxy instance
};

} // namespace core
} // namespace middleware
