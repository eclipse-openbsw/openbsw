// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/Message.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

/**
 * Abstract base class for message transceivers in the middleware.
 * This class defines the core functionality for objects that can send and receive
 * messages in the middleware layer. Transceivers represent communication endpoints (proxies or
 * skeletons) and are identified by service ID, instance ID, and address ID. They handle incoming
 * messages and can send messages to other transceivers.
 */
class TransceiverBase
{
public:
    /** Returns the service instance ID. */
    uint16_t getInstanceId() const { return _instanceId; }

    /** Returns the service ID. */
    virtual uint16_t getServiceId() const = 0;

    /** Returns the address ID used for routing messages to specific proxy instances. */
    virtual uint8_t getAddressId() const = 0;

    /**
     * Handle reception of a new message.
     * Called when a new message is received by this transceiver. Implementations should
     * process the message according to their specific logic.
     *
     * \param msg constant reference to the received message
     * \return HRESULT indicating success or failure of message processing
     */
    virtual HRESULT onNewMessageReceived(Message const& msg) = 0;

    /** Sets the service instance ID to \p instanceId. */
    void setInstanceId(uint16_t const instanceId) { _instanceId = instanceId; }

    /** Sets the address ID to \p addressId. */
    virtual void setAddressId(uint8_t addressId) = 0;

    /**
     * Check if the transceiver is initialized.
     * Returns whether this transceiver has been properly initialized and is ready to
     * send and receive messages.
     *
     * \return true if initialized, false otherwise
     */
    virtual bool isInitialized() const = 0;

    /**
     * Send a message through this transceiver.
     * Transmits the given message to the appropriate destination based on the message
     * header information.
     *
     * \param msg reference to the message to send
     * \return HRESULT indicating success or failure of the send operation
     */
    virtual HRESULT sendMessage(Message& msg) const = 0;

    /** Returns the source cluster ID of this transceiver. */
    virtual uint8_t getSourceClusterId() const = 0;

    TransceiverBase(TransceiverBase const&)            = delete;
    TransceiverBase& operator=(TransceiverBase const&) = delete;
    TransceiverBase(TransceiverBase&&)                 = delete;
    TransceiverBase& operator=(TransceiverBase&&)      = delete;

protected:
    /** The service instance ID for this transceiver. */
    uint16_t _instanceId;

    constexpr explicit TransceiverBase(uint16_t const instanceId = INVALID_INSTANCE_ID)
    : _instanceId(instanceId)
    {}

    virtual ~TransceiverBase() = default;
};

} // namespace core
} // namespace middleware
