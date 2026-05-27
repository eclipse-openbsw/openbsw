/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/common/DoIpConstants.h"
#include "doip/common/IDoIpSendJob.h"
#include "doip/common/IDoIpSendJobCallback.h"

namespace transport
{
class TransportMessage;
class ITransportMessageProcessedListener;
} // namespace transport

namespace doip
{
/**
 * DoIP send job for transmission of a transport (diagnostic) message.
 */
class DoIpTransportMessageSendJob : public IDoIpSendJob
{
public:
    using CallbackType = IDoIpSendJobCallback<DoIpTransportMessageSendJob>;

    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param message transport message to send
     * \param notificationListener optional listener to be notified when message has been sent
     * \param callback reference to callback interface
     */
    DoIpTransportMessageSendJob(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::transport::TransportMessage& message,
        ::transport::ITransportMessageProcessedListener* notificationListener,
        CallbackType& callback);

    /**
     * Constructor with deriving source addresses.
     * \param protocolVersion doip protocol version used for all communication
     * \param message transport message to send
     * \param notificationListener optional listener to be notified when message has been sent
     * \param sourceAddress source address to use for sending
     * \param targetAddress target address to use for sending
     * \param callback reference to callback interface
     */
    DoIpTransportMessageSendJob(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::transport::TransportMessage& message,
        ::transport::ITransportMessageProcessedListener* notificationListener,
        uint16_t sourceAddress,
        uint16_t targetAddress,
        CallbackType& callback);

    /**
     * Get the represented transport message.
     * \return reference to the message
     */
    ::transport::TransportMessage& getMessage();
    /**
     * Get the optional notification listener.
     * \return pointer to notification listener, may be nullptr
     */
    ::transport::ITransportMessageProcessedListener* getNotificationListener() const;
    /**
     * Get the source address.
     * \return source address
     */
    uint16_t getSourceAddress() const;
    /**
     * Get the target address.
     * \return target address
     */
    uint16_t getTargetAddress() const;

    /**
     * Send transport message processed if notification listener is given.
     * \param success true if the message has been processed successfully
     */
    void sendTransportMessageProcessed(bool success);

    uint8_t getSendBufferCount() const override;
    uint16_t getTotalLength() const override;
    ::ip::IPEndpoint const* getDestinationEndpoint() const override;
    void release(bool success) override;
    ::etl::span<uint8_t const>
    getSendBuffer(::etl::span<uint8_t> staticBuffer, uint8_t bufferIndex) override;

private:
    enum class BufferIndex : uint8_t
    {
        HEADER,
        STATIC_PAYLOAD,
        DYNAMIC_PAYLOAD,
        COUNT,
    };

    CallbackType& _callback;
    ::transport::TransportMessage& _message;
    ::transport::ITransportMessageProcessedListener* _notificationListener;
    uint16_t _sourceAddress;
    uint16_t _targetAddress;
    DoIpConstants::ProtocolVersion _protocolVersion;
};

/**
 * Inline implementations.
 */
inline ::transport::TransportMessage& DoIpTransportMessageSendJob::getMessage() { return _message; }

inline ::transport::ITransportMessageProcessedListener*
DoIpTransportMessageSendJob::getNotificationListener() const
{
    return _notificationListener;
}

inline uint16_t DoIpTransportMessageSendJob::getSourceAddress() const { return _sourceAddress; }

inline uint16_t DoIpTransportMessageSendJob::getTargetAddress() const { return _targetAddress; }

} // namespace doip
