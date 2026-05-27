/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/span.h>

namespace doip
{
class IDoIpUdpOemMessageHandler
{
public:
    virtual ~IDoIpUdpOemMessageHandler()                             = default;
    virtual void createResponse(::etl::span<uint8_t> response) const = 0;
    virtual uint16_t getRequestPayloadType() const                   = 0;
    virtual uint8_t getRequestPayloadSize() const                    = 0;
    virtual uint16_t getResponsePayloadType() const                  = 0;
    virtual uint8_t getResponsePayloadSize() const                   = 0;
    virtual bool onPayloadReceived(::etl::span<uint8_t const> payload, uint8_t& nackCode) const = 0;

private:
    IDoIpUdpOemMessageHandler& operator=(IDoIpUdpOemMessageHandler const&) = default;
};

template<
    uint16_t RequestPayloadType,
    uint8_t RequestPayloadSize,
    uint16_t ResponsePayloadType,
    uint8_t ResponsePayloadSize>
class DoIpConcreteOemMessageHandler : public IDoIpUdpOemMessageHandler
{
public:
    void createResponse(::etl::span<uint8_t> /*response*/) const override{};

    inline uint16_t getRequestPayloadType() const final { return RequestPayloadType; }

    inline uint8_t getRequestPayloadSize() const final { return RequestPayloadSize; }

    inline uint16_t getResponsePayloadType() const final { return ResponsePayloadType; }

    inline uint8_t getResponsePayloadSize() const final { return ResponsePayloadSize; }

    bool
    onPayloadReceived(::etl::span<uint8_t const> /*payload*/, uint8_t& /*nackCode*/) const override
    {
        return true;
    }
};
} // namespace doip
