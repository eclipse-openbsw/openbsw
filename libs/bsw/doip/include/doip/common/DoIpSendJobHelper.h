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

#include <etl/span.h>

namespace doip
{
/**
 * Simple helper class for common send job functionality.
 */
class DoIpSendJobHelper
{
public:
    /**
     * Prepare a send buffer for a DoIP Header.
     * \param staticBuffer reference to static buffer that will be used
     * \param protocolVersion DoIP protocol version to use
     * \param payloadType payload type
     * \param payloadLength payloadLength
     */
    static ::etl::span<uint8_t const> prepareHeaderBuffer(
        ::etl::span<uint8_t> staticBuffer,
        uint8_t protocolVersion,
        uint16_t payloadType,
        uint32_t payloadLength);
};

} // namespace doip
