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

#include <platform/estdint.h>

namespace doip
{
/**
 * Class holding all needed parameters for the DoIP server transport layer.
 */
class DoIpServerTransportLayerParameters
{
public:
    /**
     * Constructor.
     * \param initialInactivityTimeout timeout for reception of a routing activation request
     * \param generalInactivityTimeout timeout for maximum inactivity of a connection
     * \param aliveCheckTimeout timeout for a response to a alive check request
     * \param maxPayloadLength maximum payload length of a doip message
     */
    DoIpServerTransportLayerParameters(
        uint16_t initialInactivityTimeout,
        uint32_t generalInactivityTimeout,
        uint16_t aliveCheckTimeout,
        uint32_t maxPayloadLength);

    /**
     * Get timeout for reception of a routing activation request
     * \return timeout (ms)
     */
    uint16_t getInitialInactivityTimeout() const;
    /**
     * Get timeout for maximum inactivity of a connection
     * \return timeout (ms)
     */
    uint32_t getGeneralInactivityTimeout() const;
    /**
     * Get timeout for a response to a alive check request
     * \return timeout (ms)
     */
    uint16_t getAliveCheckTimeout() const;
    /**
     * Get maximum payload length of a doip message
     * \return length
     */
    uint32_t getMaxPayloadLength() const;

private:
    uint32_t _generalInactivityTimeout;
    uint16_t _initialInactivityTimeout;
    uint16_t _aliveCheckTimeout;
    uint32_t _maxPayloadLength;
};

/**
 * Inline implementations.
 */
inline DoIpServerTransportLayerParameters::DoIpServerTransportLayerParameters(
    uint16_t const initialInactivityTimeout,
    uint32_t const generalInactivityTimeout,
    uint16_t const aliveCheckTimeout,
    uint32_t const maxPayloadLength)
: _generalInactivityTimeout(generalInactivityTimeout)
, _initialInactivityTimeout(initialInactivityTimeout)
, _aliveCheckTimeout(aliveCheckTimeout)
, _maxPayloadLength(maxPayloadLength)
{}

inline uint16_t DoIpServerTransportLayerParameters::getInitialInactivityTimeout() const
{
    return _initialInactivityTimeout;
}

inline uint32_t DoIpServerTransportLayerParameters::getGeneralInactivityTimeout() const
{
    return _generalInactivityTimeout;
}

inline uint16_t DoIpServerTransportLayerParameters::getAliveCheckTimeout() const
{
    return _aliveCheckTimeout;
}

inline uint32_t DoIpServerTransportLayerParameters::getMaxPayloadLength() const
{
    return _maxPayloadLength;
}

} // namespace doip
