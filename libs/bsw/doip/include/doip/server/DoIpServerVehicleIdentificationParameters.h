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

#include <doip/server/IDoIpServerVehicleAnnouncementParameterProvider.h>

#include <platform/estdint.h>

namespace doip
{
/**
 * Class that holds parameters for vehicle identification service.
 */
class DoIpServerVehicleIdentificationParameters
: public IDoIpServerVehicleAnnouncementParameterProvider
{
public:
    /**
     * Constructor.
     * \param announceWaitTimeout time (in ms) to wait for responding to first announcement request
     * or sending broadcast \param announceInterval time (in ms) between sending identification
     * responses after IP address if configured \param announceCount number of times to send
     * identification responses after IP address if configured
     */
    DoIpServerVehicleIdentificationParameters(
        uint16_t announceWaitTimeout, uint16_t announceInterval, uint8_t announceCount);

    /**
     * Get time to wait for responding to first announcement request or sending broadcast.
     * \return time in ms
     */
    uint16_t getAnnounceWait() const override;

    /**
     * Get time between sending identification responses after IP address if configured.
     * \return time in ms
     */
    uint16_t getAnnounceInterval() const override;

    /**
     * Get the number of vehicle identifications to send after IP address is configured.
     * \return number of messages to send
     */
    uint8_t getAnnounceCount() const;

private:
    uint16_t _announceWaitTimeout;
    uint16_t _announceInterval;
    uint8_t _announceCount;
};

/**
 * Implementations.
 */
inline DoIpServerVehicleIdentificationParameters::DoIpServerVehicleIdentificationParameters(
    uint16_t const announceWaitTimeout,
    uint16_t const announceInterval,
    uint8_t const announceCount)
: _announceWaitTimeout(announceWaitTimeout)
, _announceInterval(announceInterval)
, _announceCount(announceCount)
{}

inline uint16_t DoIpServerVehicleIdentificationParameters::getAnnounceWait() const
{
    return _announceWaitTimeout;
}

inline uint16_t DoIpServerVehicleIdentificationParameters::getAnnounceInterval() const
{
    return _announceInterval;
}

inline uint8_t DoIpServerVehicleIdentificationParameters::getAnnounceCount() const
{
    return _announceCount;
}

} // namespace doip
