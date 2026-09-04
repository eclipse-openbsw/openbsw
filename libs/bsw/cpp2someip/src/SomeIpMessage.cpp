/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpMessage.h"

#include "someip/SomeIpConstants.h"
#include "someip/endian_helpers.h"

#include <etl/algorithm.h>
#include <etl/array.h>
#include <util/meta/Bitmask.h>

namespace someip
{
using configuration::NUMBER_OF_BITS_CLIENT_ID;
using ::util::meta::Bitmask;

namespace
{
::etl::array<uint8_t const, SomeIpConstants::HEADER_LENGTH> const clientToServerCookieBuffer{
    {0xFFU,
     0xFFU,
     0x00U,
     0x00U,
     0x00U,
     0x00U,
     0x00U,
     0x08U,
     0xDEU,
     0xADU,
     0xBEU,
     0xEFU,
     0x01U,
     0x01U,
     0x01U,
     0x00U}};
::etl::array<uint8_t const, SomeIpConstants::HEADER_LENGTH> const serverToClientCookieBuffer{
    {0xFFU,
     0xFFU,
     0x80U,
     0x00U,
     0x00U,
     0x00U,
     0x00U,
     0x08U,
     0xDEU,
     0xADU,
     0xBEU,
     0xEFU,
     0x01U,
     0x01U,
     0x02U,
     0x00U}};
} // namespace

uint32_t readLength(::etl::span<uint8_t const> const& messageBuffer)
{
    return ::someip::endian::read_be<uint32_t>(&messageBuffer[SomeIpMessage::OFFSET_LENGTH]);
}

uint32_t readTotalLength(::etl::span<uint8_t const> const& messageBuffer)
{
    return readLength(messageBuffer)
           + (SomeIpMessage::OFFSET_PAYLOAD - SomeIpMessage::OFFSET_REQUEST_ID);
}

::etl::span<uint8_t const> SomeIpMessage::getBufferHeader() const
{
    return _buffer.first(SomeIpMessage::OFFSET_PAYLOAD);
}

::etl::span<uint8_t const> SomeIpMessage::getBufferPayload() const
{
    return _buffer.subspan(OFFSET_PAYLOAD, getPayloadLength());
}

uint8_t const* SomeIpMessage::getPayload() const
{
    if (_buffer.size() <= OFFSET_PAYLOAD)
    {
        return nullptr;
    }
    return &_buffer[OFFSET_PAYLOAD];
}

uint8_t* SomeIpMessage::getPayload()
{
    if (_buffer.size() <= OFFSET_PAYLOAD)
    {
        return nullptr;
    }
    return &_buffer[OFFSET_PAYLOAD];
}

uint32_t SomeIpMessage::getMessageId() const
{
    return ::someip::endian::read_be<uint32_t>(&_buffer[OFFSET_MESSAGE_ID]);
}

void SomeIpMessage::setMessageId(uint32_t const messageId)
{
    ::someip::endian::write_be<uint32_t>(&_buffer[OFFSET_MESSAGE_ID], messageId);
}

service_id::type SomeIpMessage::getServiceId() const
{
    return ::someip::endian::read_be<uint16_t>(&_buffer[OFFSET_SERVICE_ID]);
}

void SomeIpMessage::setServiceId(service_id::type const serviceId)
{
    ::someip::endian::write_be<uint16_t>(&_buffer[OFFSET_SERVICE_ID], serviceId);
}

uint16_t SomeIpMessage::getMethodId() const
{
    return ::someip::endian::read_be<uint16_t>(&_buffer[OFFSET_METHOD_ID]);
}

void SomeIpMessage::setMethodId(uint16_t const methodId)
{
    ::someip::endian::write_be<uint16_t>(&_buffer[OFFSET_METHOD_ID], methodId);
}

void SomeIpMessage::setLength(uint32_t const length)
{
    ::someip::endian::write_be<uint32_t>(&_buffer[OFFSET_LENGTH], length);
}

uint32_t SomeIpMessage::getRequestId() const
{
    return ::someip::endian::read_be<uint32_t>(&_buffer[OFFSET_REQUEST_ID]);
}

void SomeIpMessage::setRequestId(uint32_t const requestId)
{
    ::someip::endian::write_be<uint32_t>(&_buffer[OFFSET_REQUEST_ID], requestId);
}

uint16_t SomeIpMessage::getClientId() const
{
    return static_cast<uint16_t>(
        getRequestId() >> (32U - static_cast<uint32_t>(NUMBER_OF_BITS_CLIENT_ID)));
}

void SomeIpMessage::setClientId(uint16_t const clientId)
{
    uint32_t tmp = clientId;
    tmp <<= (32U - static_cast<uint32_t>(NUMBER_OF_BITS_CLIENT_ID));
    tmp |= static_cast<uint32_t>(getSessionId());
    setRequestId(tmp);
}

uint16_t SomeIpMessage::getSessionId() const
{
    return static_cast<uint16_t>(
        getRequestId()
        & Bitmask<uint32_t, 32U - static_cast<uint32_t>(NUMBER_OF_BITS_CLIENT_ID)>::value);
}

void SomeIpMessage::setSessionId(uint16_t const sessionId)
{
    uint32_t tmp = getClientId();
    tmp <<= (32U - static_cast<uint32_t>(NUMBER_OF_BITS_CLIENT_ID));
    tmp |= static_cast<uint32_t>(sessionId);
    setRequestId(tmp);
}

uint8_t SomeIpMessage::getProtocolVersion() const { return _buffer[OFFSET_PROTOCOL_VERSION]; }

void SomeIpMessage::setProtocolVersion(uint8_t const protocolVersion)
{
    _buffer[OFFSET_PROTOCOL_VERSION] = protocolVersion;
}

uint8_t SomeIpMessage::getInterfaceVersion() const { return _buffer[OFFSET_INTERFACE_VERSION]; }

void SomeIpMessage::setInterfaceVersion(uint8_t const interfaceVersion)
{
    _buffer[OFFSET_INTERFACE_VERSION] = interfaceVersion;
}

SomeIpMessage::MessageType SomeIpMessage::getMessageType() const
{
    return static_cast<MessageType>(_buffer[OFFSET_MESSAGE_TYPE]);
}

void SomeIpMessage::setMessageType(MessageType const messageType)
{
    _buffer[OFFSET_MESSAGE_TYPE] = static_cast<uint8_t>(messageType);
}

uint8_t SomeIpMessage::getRawMessageType() const { return _buffer[OFFSET_MESSAGE_TYPE]; }

void SomeIpMessage::setRawMessageType(uint8_t const messageType)
{
    _buffer[OFFSET_MESSAGE_TYPE] = messageType;
}

SomeIpMessage::ReturnCode SomeIpMessage::getReturnCode() const
{
    return static_cast<SomeIpMessage::ReturnCode>(_buffer[OFFSET_RETURN_CODE]);
}

void SomeIpMessage::setReturnCode(SomeIpMessage::ReturnCode const returnCode)
{
    _buffer[OFFSET_RETURN_CODE] = static_cast<uint8_t>(returnCode);
}

uint32_t SomeIpMessage::getPayloadLength() const
{
    uint32_t const lengthCorrection = OFFSET_PAYLOAD - OFFSET_REQUEST_ID;
    uint32_t const length           = getLength();
    if (length >= lengthCorrection)
    {
        return length - lengthCorrection;
    }

    return 0U;
}

void SomeIpMessage::setPayloadLength(uint32_t const payloadLength)
{
    setLength(payloadLength + (OFFSET_PAYLOAD - OFFSET_REQUEST_ID));
}

uint32_t SomeIpMessage::getMaximumPayloadLength() const
{
    return static_cast<uint32_t>(_buffer.size()) - OFFSET_PAYLOAD;
}

uint8_t SomeIpMessage::getFlags() const { return getPayload()[OFFSET_SD_FLAGS]; }

void SomeIpMessage::makeClientToServerMagicCookieMessage(SomeIpMessage& message)
{
    etl::copy(
        clientToServerCookieBuffer.begin(),
        clientToServerCookieBuffer.end(),
        message._buffer.begin());
}

void SomeIpMessage::makeServerToClientMagicCookieMessage(SomeIpMessage& message)
{
    etl::copy(
        serverToClientCookieBuffer.begin(),
        serverToClientCookieBuffer.end(),
        message._buffer.begin());
}

} // namespace someip
