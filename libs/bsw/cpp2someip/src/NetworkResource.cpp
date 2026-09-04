/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkResource.h"

namespace someip
{
// NOLINTNEXTLINE(cert-err58-cpp): Fine, since default value shall be used.
::ip::IPAddress const NetworkResource::INVALID_IP = {{0U}};
// NOLINTNEXTLINE(cert-err58-cpp): Fine, since default value shall be used.
::ip::IPEndpoint const NetworkResource::INVALID_ADDRESS;

bool NetworkResource::isInitialized() const
{
    return (_pInputBuffer.size() != 0U) && (_pOutputBuffer.size() != 0U) && (_pListener != nullptr);
}

::etl::span<uint8_t> NetworkResource::getInputBuffer() const
{
    if (!isInitialized())
    {
        return {};
    }

    return _pInputBuffer;
}

::etl::span<uint8_t> NetworkResource::getOutputBuffer() const
{
    if (!isInitialized())
    {
        return {};
    }

    return _pOutputBuffer;
}

bool NetworkResource::send(
    ::ip::IPEndpoint const& /* remoteEndpoint */, uint32_t const /* length */)
{
    return false;
}

bool NetworkResource::send(uint32_t const /* length */) { return false; }

} // namespace someip
