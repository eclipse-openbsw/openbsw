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

#include <etl/type_traits.h>
#include <util/types/Enum.h>
#include <platform/estdint.h>

namespace doip
{
/**
 * Common constants
 */
struct DoIpConstants
{
    enum class ProtocolVersion : uint8_t
    {
        version02Iso2012 = 0x02U,
        version03Iso2019 = 0x03U,
    };
    static uint32_t const DOIP_HEADER_LENGTH = 8U;

    enum class DiagnosticPowerMode : uint8_t
    {
        NOT_READY     = 0x00U,
        READY         = 0x01U,
        NOT_SUPPORTED = 0x02U
    };

    struct Timings
    {
        static uint16_t const DOIP_ANNOUNCE_INTERVAL_MS = 500U;
    };

    struct Ports
    {
        static uint16_t const UDP_DISCOVERY = 13400U;
        static uint16_t const TCP_DATA      = 13400U;
        static uint16_t const TCP_DATA_TLS  = 3496U;
    };

    struct PayloadTypes
    {
        static uint16_t const NEGATIVE_ACK                               = 0x0000U;
        static uint16_t const VEHICLE_IDENTIFICATION_REQUEST_MESSAGE     = 0x0001U;
        static uint16_t const VEHICLE_IDENTIFICATION_REQUEST_MESSAGE_EID = 0x0002U;
        static uint16_t const VEHICLE_IDENTIFICATION_REQUEST_MESSAGE_VIN = 0x0003U;
        static uint16_t const VEHICLE_ANNOUNCEMENT_MESSAGE               = 0x0004U;
        static uint16_t const ROUTING_ACTIVATION_REQUEST                 = 0x0005U;
        static uint16_t const ROUTING_ACTIVATION_RESPONSE                = 0x0006U;
        static uint16_t const ALIVE_CHECK_REQUEST                        = 0x0007U;
        static uint16_t const ALIVE_CHECK_RESPONSE                       = 0x0008U;

        static uint16_t const ENTITY_STATUS_REQUEST                      = 0x4001U;
        static uint16_t const ENTITY_STATUS_RESPONSE                     = 0x4002U;
        static uint16_t const DIAGNOSTIC_POWER_MODE_INFORMATION_REQUEST  = 0x4003U;
        static uint16_t const DIAGNOSTIC_POWER_MODE_INFORMATION_RESPONSE = 0x4004U;

        static uint16_t const DIAGNOSTIC_MESSAGE              = 0x8001U;
        static uint16_t const DIAGNOSTIC_MESSAGE_POSITIVE_ACK = 0x8002U;
        static uint16_t const DIAGNOSTIC_MESSAGE_NEGATIVE_ACK = 0x8003U;

        static uint16_t const OEM_REQUEST_MIN = 0xF000U;
        static uint16_t const OEM_REQUEST_MAX = 0xFFFFU;
    };

    static bool isValidOemRequest(uint16_t const payloadType)
    {
        return (
            (payloadType >= DoIpConstants::PayloadTypes::OEM_REQUEST_MIN)
            && (payloadType <= DoIpConstants::PayloadTypes::OEM_REQUEST_MAX));
    }

    struct NackCodes
    {
        static uint8_t const NACK_INCORRECT_PATTERN      = 0x00U;
        static uint8_t const NACK_UNKNOWN_PAYLOAD_TYPE   = 0x01U;
        static uint8_t const NACK_MESSAGE_TOO_LARGE      = 0x02U;
        static uint8_t const NACK_OUT_OF_MEMORY          = 0x03U;
        static uint8_t const NACK_INVALID_PAYLOAD_LENGTH = 0x04U;
    };

    struct DiagnosticMessageNackCodes
    {
        static uint8_t const NACK_DIAG_SUCCESS                      = 0x00U;
        static uint8_t const NACK_DIAG_INVALID_SOURCE_ADDRESS       = 0x02U;
        static uint8_t const NACK_DIAG_INVALID_TARGET_ADDRESS       = 0x03U;
        static uint8_t const NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE = 0x04U;
        static uint8_t const NACK_DIAG_OUT_OF_MEMORY                = 0x05U;
        static uint8_t const NACK_DIAG_TARGET_UNREACHABLE           = 0x06U;
        static uint8_t const NACK_DIAG_UNKNOWN_NETWORK              = 0x07U;
        static uint8_t const NACK_DIAG_TRANSPORT_PROTOCOL_ERROR     = 0x08U;
    };

    struct RoutingResponseCodes
    {
        static uint8_t const ROUTING_UNKNOWN_SOURCE_ADDRESS      = 0x00U;
        static uint8_t const ROUTING_NO_FREE_SOCKET              = 0x01U;
        static uint8_t const ROUTING_WRONG_SOURCE_ADDRESS        = 0x02U;
        static uint8_t const ROUTING_SOURCE_ALREADY_REGISTERED   = 0x03U;
        static uint8_t const ROUTING_MISSING_AUTHENTICATION      = 0x04U;
        static uint8_t const ROUTING_REJECTED_CONFIRMATION       = 0x05U;
        static uint8_t const ROUTING_UNSUPPORTED_ACTIVATION_TYPE = 0x06U;
        static uint8_t const ROUTING_SUCCESS                     = 0x10U;
        static uint8_t const ROUTING_CONFIRMATION_REQUIRED       = 0x11U;
    };

    struct EntityStatusNodeType
    {
        static uint8_t const NODE_TYPE_GATEWAY = 0x00U;
        static uint8_t const NODE_TYPE_NODE    = 0x01U;
    };
};

} // namespace doip
