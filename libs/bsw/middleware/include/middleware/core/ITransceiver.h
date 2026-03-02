// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

/**
 * \brief Interface for message transceivers in the middleware.
 * \details This interface defines the core functionality for objects that can send and receive
 * messages in the middleware layer. Transceivers represent communication endpoints (proxies or
 * skeletons) and are identified by service ID, instance ID, and address ID. They handle incoming
 * messages and can send messages to other transceivers.
 */
class ITransceiver
{
public:
    /**
     * \brief Get the service instance ID.
     * \return the service instance ID
     */
    uint16_t getInstanceId() const { return instanceId_; }

    /**
     * \brief Get the service ID.
     * \return the service ID
     */
    virtual uint16_t getServiceId() const = 0;

    /**
     * \brief Get the address ID.
     * \details Returns the unique address identifier for this transceiver instance, used for
     * routing messages to specific proxy instances.
     *
     * \return the address ID
     */
    virtual uint8_t getAddressId() const = 0;

    /**
     * \brief Handle reception of a new message.
     * \details Called when a new message is received by this transceiver. Implementations should
     * process the message according to their specific logic.
     *
     * \param msg constant reference to the received message
     * \return HRESULT indicating success or failure of message processing
     */
    virtual HRESULT onNewMessageReceived(Message const& msg) = 0;

    /**
     * \brief Set the service instance ID.
     * \param instanceId the new service instance ID to set
     */
    void setInstanceId(uint16_t const instanceId) { instanceId_ = instanceId; }

    /**
     * \brief Set the address ID.
     * \details Updates the unique address identifier for this transceiver instance.
     *
     * \param addressId the new address ID to set
     */
    virtual void setAddressId(uint8_t const addressId) = 0;

    /**
     * \brief Check if the transceiver is initialized.
     * \details Returns whether this transceiver has been properly initialized and is ready to
     * send and receive messages.
     *
     * \return true if initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;

    /**
     * \brief Send a message through this transceiver.
     * \details Transmits the given message to the appropriate destination based on the message
     * header information.
     *
     * \param msg reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    virtual HRESULT sendMessage(Message& msg) const = 0;

    /**
     * \brief Get the source cluster ID.
     * \details Returns the identifier of the cluster where this transceiver resides.
     *
     * \return the source cluster ID
     */
    virtual uint8_t getSourceClusterId() const = 0;

    ITransceiver(ITransceiver const&)            = delete;
    ITransceiver& operator=(ITransceiver const&) = delete;
    ITransceiver(ITransceiver&&)                 = delete;
    ITransceiver& operator=(ITransceiver&&)      = delete;

protected:
    uint16_t instanceId_; ///< The service instance ID for this transceiver

    constexpr explicit ITransceiver(uint16_t const instanceId = INVALID_INSTANCE_ID)
    : instanceId_(instanceId)
    {}

    virtual ~ITransceiver() = default;
};

} // namespace core
} // namespace middleware
