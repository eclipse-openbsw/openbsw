// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32SecurityAccess.h"

#include "uds/UdsConstants.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

namespace uds
{

Stm32SecurityAccess::Stm32SecurityAccess()
: Service(ServiceId::SECURITY_ACCESS, DiagSession::ALL_SESSIONS())
, _seed(0x12345678U)
, _seedIssued(false)
, _unlocked(false)
, _failedAttempts(0U)
, _lockedUntilReset(false)
{}

void Stm32SecurityAccess::reset()
{
    _seedIssued       = false;
    _unlocked         = false;
    _failedAttempts   = 0U;
    _lockedUntilReset = false;
}

uint32_t Stm32SecurityAccess::nextSeed()
{
    _seed = (_seed * LCG_A + LCG_C) | 0x00000001U;
    return _seed;
}

DiagReturnCode::Type Stm32SecurityAccess::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    if (requestLength < 1U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    uint8_t const subFunction = request[0];

    if (subFunction == 0x01U)
    {
        return handleRequestSeed(connection);
    }
    else if (subFunction == 0x02U)
    {
        return handleSendKey(connection, request, requestLength);
    }

    return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
}

DiagReturnCode::Type Stm32SecurityAccess::handleRequestSeed(IncomingDiagConnection& connection)
{
    if (_lockedUntilReset)
    {
        return DiagReturnCode::ISO_EXCEEDED_NUMS_OF_ATTEMPTS;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(0x01U); // echo subfunction

    if (_unlocked)
    {
        // Already unlocked — return 4-byte zero seed per ISO 14229
        (void)response.appendUint8(0x00U);
        (void)response.appendUint8(0x00U);
        (void)response.appendUint8(0x00U);
        (void)response.appendUint8(0x00U);
    }
    else
    {
        uint32_t const seed = nextSeed();
        _seedIssued         = true;
        // 4-byte big-endian seed (matching Rust firmware)
        (void)response.appendUint8(static_cast<uint8_t>((seed >> 24U) & 0xFFU));
        (void)response.appendUint8(static_cast<uint8_t>((seed >> 16U) & 0xFFU));
        (void)response.appendUint8(static_cast<uint8_t>((seed >> 8U) & 0xFFU));
        (void)response.appendUint8(static_cast<uint8_t>(seed & 0xFFU));
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

DiagReturnCode::Type Stm32SecurityAccess::handleSendKey(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    if (_lockedUntilReset)
    {
        return DiagReturnCode::ISO_EXCEEDED_NUMS_OF_ATTEMPTS;
    }

    if (!_seedIssued)
    {
        return DiagReturnCode::ISO_REQUEST_SEQUENCE_ERROR;
    }

    if (requestLength < 3U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    uint16_t const receivedKey
        = (static_cast<uint16_t>(request[1]) << 8U) | static_cast<uint16_t>(request[2]);
    uint16_t const expectedKey = static_cast<uint16_t>(_seed ^ XOR_SECRET);

    if (receivedKey == expectedKey)
    {
        _unlocked       = true;
        _seedIssued     = false;
        _failedAttempts = 0U;

        PositiveResponse& response = connection.releaseRequestGetResponse();
        (void)response.appendUint8(0x02U); // echo subfunction
        (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
        return DiagReturnCode::OK;
    }

    _failedAttempts++;
    _seedIssued = false;

    if (_failedAttempts >= MAX_FAILED_ATTEMPTS)
    {
        _lockedUntilReset = true;
        return DiagReturnCode::ISO_EXCEEDED_NUMS_OF_ATTEMPTS;
    }

    return DiagReturnCode::ISO_INVALID_KEY;
}

} // namespace uds
