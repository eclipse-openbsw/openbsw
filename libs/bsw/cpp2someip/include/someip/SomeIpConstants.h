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

#include <cstddef>
#include <cstdint>

namespace someip
{
namespace configuration
{
uint8_t const PROTOCOL_VERSION          = 0x01U;
uint8_t const INTERFACE_VERSION         = 0x01U;
uint8_t const NUMBER_OF_BITS_SERVICE_ID = 16U;
uint8_t const NUMBER_OF_BITS_CLIENT_ID  = 16U;

} // namespace configuration

enum class ErrorCode : uint8_t
{
    SOMEIP_OK,
    SOMEIP_ERROR,
    SOMEIP_CODING
};

enum class PortError : uint8_t
{
    NOT_INITIALIZED,
    NOT_AVAILABLE,
    OUT_OF_RANGE
};

namespace SomeIpConstants
{
static constexpr size_t HEADER_LENGTH            = 16U;
static constexpr uint16_t HEADER_2ND_HALF_LENGTH = 8U;
static constexpr uint8_t INVALID_PROTO           = 0xFFU;
} // namespace SomeIpConstants

enum class SomeIpMethodType : uint16_t
{
    METHOD_UNKNOWN_METHOD,
    METHOD_NO_PARAMETERS,
    METHOD_REQUEST_RESPONSE,
    METHOD_REQUEST_ONLY,
    METHOD_RESPONSE_ONLY
};

enum class SomeIpCallSemantic : uint16_t
{
    SEMANTIC_UNKNOWN,
    SEMANTIC_REQUEST_RESPONSE,
    SEMANTIC_FIRE_AND_FORGET
};

enum class SomeIpEventType : uint16_t
{
    EVENT_TYPE_METHOD,
    EVENT_TYPE_FIELD
};

enum class SomeIpMessageConstants : uint64_t
{
    LENGTH_FIELD_LENGTH = 4U
};

struct service_id
{
    using type = uint16_t;

    enum : type
    {
        INVALID = 0xFFFFU
    };

    service_id()                             = delete;
    service_id(service_id const&)            = delete;
    service_id& operator=(service_id const&) = delete;
};

struct instance_id
{
    using type = uint16_t;

    enum : type
    {
        ANY = 0xFFFFU
    };

    instance_id()                              = delete;
    instance_id(instance_id const&)            = delete;
    instance_id& operator=(instance_id const&) = delete;
};

struct eventgroup_id
{
    using type = uint16_t;

    enum : type
    {
        ALL = 0xFFFFU
    };

    eventgroup_id()                                = delete;
    eventgroup_id(eventgroup_id const&)            = delete;
    eventgroup_id& operator=(eventgroup_id const&) = delete;
};

struct major_version
{
    using type = uint8_t;

    enum : type
    {
        INVALID = 0xFFU,
        ANY     = INVALID
    };

    major_version()                                = delete;
    major_version(major_version const&)            = delete;
    major_version& operator=(major_version const&) = delete;
};

struct minor_version
{
    using type = uint32_t;

    enum : type
    {
        INVALID = 0xFFFFFFFFU,
        ANY     = INVALID
    };

    minor_version()                                = delete;
    minor_version(minor_version const&)            = delete;
    minor_version& operator=(minor_version const&) = delete;
};

struct ttl
{
    using type = uint32_t;

    enum : type
    {
        INVALID = 0xFFFFFFFFU
    };

    ttl()                      = delete;
    ttl(ttl const&)            = delete;
    ttl& operator=(ttl const&) = delete;
};

struct port
{
    using type = uint16_t;

    enum : type
    {
        INVALID   = 0U,
        MULTICAST = 30490U
    };

    port()                       = delete;
    port(port const&)            = delete;
    port& operator=(port const&) = delete;
};

struct proto
{
    using type = uint8_t;

    enum : type
    {
        SD_L4_PROTO_TCP = 0x06U,
        SD_L4_PROTO_UDP = 0x11U,
    };

    proto()                        = delete;
    proto(proto const&)            = delete;
    proto& operator=(proto const&) = delete;
};

enum class ServiceDiscoveryConstants : uint32_t
{
    SD_MESSAGE_TYPE   = 0x02,
    SD_MULTICAST_PORT = 30490
};

uint8_t const ENTRY_TYPE_FIND            = 0U;
uint8_t const ENTRY_TYPE_OFFER           = 1U;
uint8_t const ENTRY_TYPE_REQUEST         = 2U;
uint8_t const ENTRY_TYPE_FIND_EVENTGROUP = 4U;
uint8_t const ENTRY_TYPE_PUBLISH         = 5U;
uint8_t const ENTRY_TYPE_SUBSCRIBE       = 6U;
uint8_t const ENTRY_TYPE_SUBSCRIBE_ACK   = 7U;

enum class ServiceDiscoveryOptionType : uint8_t
{
    OPTION_TYPE_IP4_ENDPOINT  = 0x04,
    OPTION_TYPE_IP4_MULTICAST = 0x14,
    OPTION_TYPE_IP6_ENDPOINT  = 0x06,
    OPTION_TYPE_IP6_MULTICAST = 0x16
};

enum class PortRangeReturnCode : uint8_t
{
    OK,
    WARNING_INVALID_CURRENT_PORT,
    ERROR_REQUESTED_PORT_OUT_OF_RANGE
};

uint32_t const SD_MESSAGE_ID        = 0xFFFF8100U;
uint32_t const INVALID_PDU_ID       = 0xFFFFFFFFU;
uint32_t const SD_PACKET_MAX_SIZE   = 1416U;
uint16_t const EVENT_METHOD_ID_MASK = 0x8000U;

uint32_t const MAGIC_COOKIE_CLIENT_MESSAGE_ID = 0xFFFF0000U; // client -> server
uint32_t const MAGIC_COOKIE_SERVER_MESSAGE_ID = 0xFFFF8000U; // server -> client
uint32_t const MAGIC_COOKIE_REQUEST_ID        = 0xDEADBEEFU;

bool const UDP_SAVE_MODE           = true; /* support additional headers */
uint32_t const UDP_PACKET_MAX_SIZE = 1416U;

uint32_t const UDP_PAYLOAD_MAX_SIZE = (UDP_PACKET_MAX_SIZE - 16U /* someip header */);

uint32_t const TP_PAYLOAD_MAX_SIZE
    = ((UDP_PACKET_MAX_SIZE - 20U /* someip+tp header */) & 0xFFFFFFF0U /* multiple of 16 */);

} // namespace someip
