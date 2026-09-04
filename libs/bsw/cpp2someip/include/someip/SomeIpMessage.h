/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/SomeIpConstants.h"

#include <etl/span.h>

#include <cstdint>

namespace someip
{
/*
 * \return Length of a message, without header.
 */
uint32_t readLength(::etl::span<uint8_t const> const& messageBuffer);

/**
 * \return Total length of message including header.
 */
uint32_t readTotalLength(::etl::span<uint8_t const> const& messageBuffer);

/**
 * Class representing a SOME/IP Message and providing methods for its
 * serialization.
 *
 * \section header  SomeIp Header
 * \li  32 bit: Message ID (Service ID / Method ID)
 * \li  32 bit: Length
 * \li  32 bit: Request ID (Client ID / Session ID)
 * \li   8 bit: Protocol Version
 * \li   8 bit: Interface Version
 * \li   8 bit: Message Type
 * \li   8 bit: Return Code
 */
class SomeIpMessage
{
public:
    static uint32_t const OFFSET_MESSAGE_ID        = 0U;
    static uint32_t const OFFSET_SERVICE_ID        = 0U;
    static uint32_t const OFFSET_METHOD_ID         = 2U;
    static uint32_t const OFFSET_LENGTH            = 4U;
    static uint32_t const OFFSET_REQUEST_ID        = 8U;
    static uint32_t const OFFSET_CLIENT_ID         = 8U;
    static uint32_t const OFFSET_SESSION_ID        = 10U;
    static uint32_t const OFFSET_PROTOCOL_VERSION  = 12U;
    static uint32_t const OFFSET_INTERFACE_VERSION = 13U;
    static uint32_t const OFFSET_MESSAGE_TYPE      = 14U;
    static uint32_t const OFFSET_RETURN_CODE       = 15U;
    static uint32_t const OFFSET_PAYLOAD           = 16U;
    static uint32_t const OFFSET_SD_FLAGS          = 0U;

    /** Possible types of a SOME/IP message */
    enum class MessageType : uint8_t
    {
        REQUEST           = 0x00,
        REQUEST_NO_RETURN = 0x01,
        NOTIFICATION      = 0x02,
        RESPONSE          = 0x80,
        EXCEPTION         = 0x81
    };

    enum class ReturnCode : uint8_t
    {
        SOMEIP_E_OK                      = 0x00,
        SOMEIP_E_NOT_OK                  = 0x01,
        SOMEIP_E_UNKNOWN_SERVICE         = 0x02,
        SOMEIP_E_UNKNOWN_METHOD          = 0x03,
        SOMEIP_E_NOT_READY               = 0x04,
        SOMEIP_E_NOT_REACHABLE           = 0x05,
        SOMEIP_E_TIMEOUT                 = 0x06,
        SOMEIP_E_WRONG_PROTOCOL_VERSION  = 0x07,
        SOMEIP_E_WRONG_INTERFACE_VERSION = 0x08,
        SOMEIP_E_MALFORMED_MESSAGE       = 0x09,
        SOMEIP_E_WRONG_MESSAGE_TYPE      = 0x0A
    };

    /**
     * Create a SOME/IP message and use the specified buffer as the data
     * for the message.
     *
     * \param buffer The buffer where the message will be read or written.
     */
    explicit SomeIpMessage(::etl::span<uint8_t> const& buffer) : _buffer(buffer) {}

    SomeIpMessage(SomeIpMessage const&)            = delete;
    SomeIpMessage& operator=(SomeIpMessage const&) = delete;

    /**
     * Returns the message id, which is composed of service id and method
     * id where 2 bytes are used as service id followed by 2 bytes for method id.
     */
    uint32_t getMessageId() const;

    /** \see getMessageId() */
    void setMessageId(uint32_t messageId);

    /** Returns the service id for this message */
    service_id::type getServiceId() const;

    /** Sets the service id to the specified id */
    void setServiceId(service_id::type serviceId);

    /** Returns the method id for this message */
    uint16_t getMethodId() const;

    /** Sets the method id to the specified id */
    void setMethodId(uint16_t methodId);

    /** Returns the length of message without header */
    uint32_t getLength() const { return readLength(_buffer); }

    /** Sets the length of this message */
    void setLength(uint32_t length);

    /**
     * aka SessionId
     */
    uint32_t getRequestId() const;
    void setRequestId(uint32_t requestId);

    uint16_t getClientId() const;
    void setClientId(uint16_t clientId);

    uint16_t getSessionId() const;
    void setSessionId(uint16_t sessionId);

    uint8_t getProtocolVersion() const;
    void setProtocolVersion(uint8_t protocolVersion);

    uint8_t getInterfaceVersion() const;
    void setInterfaceVersion(uint8_t interfaceVersion);

    MessageType getMessageType() const;
    void setMessageType(MessageType messageType);

    void setRawMessageType(uint8_t messageType);
    uint8_t getRawMessageType() const;

    ReturnCode getReturnCode() const;
    void setReturnCode(ReturnCode returnCode);

    ::etl::span<uint8_t const> getBufferHeader() const;
    ::etl::span<uint8_t const> getBufferPayload() const;

    uint8_t const* getPayload() const;

    /** Returns a pointer to the message beginning after the header */
    uint8_t* getPayload();

    /** Returns a pointer to the raw buffer containing the entire message */
    uint8_t* getRawData() const { return _buffer.data(); }

    /**
     * \return Length of payload.
     */
    uint32_t getPayloadLength() const;

    void setPayloadLength(uint32_t payloadLength);

    /**
     * \return Total length of message including header.
     */
    uint32_t getTotalLength() const { return readTotalLength(_buffer); }

    /**
     * \return Maximum length of payload.
     */
    uint32_t getMaximumPayloadLength() const;

    uint8_t getFlags() const;

    /**
     * \return Client to server magic cookie message.
     */
    static void makeClientToServerMagicCookieMessage(SomeIpMessage& message);

    /**
     * \return Server to client magic cookie message.
     */
    static void makeServerToClientMagicCookieMessage(SomeIpMessage& message);

private:
    static uint32_t const INVALID_VALUE = 0xFFFFFFFFU;

    ::etl::span<uint8_t> _buffer;
};
} // namespace someip
