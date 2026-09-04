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

#include "someip/ITpListener.h"
#include "someip/SomeIpMessage.h"

#include <etl/array.h>
#include <etl/span.h>

#include <cstdint>

namespace someip
{
/**
 * Implementation of a TP receiver.
 */
class TpReceiver
{
public:
    enum class TpResult : uint8_t
    {
        TP_OK,
        TP_PENDING,
        TP_ERROR
    };

    bool isActive() const;

    bool isExpired(uint32_t time) const;

    bool isMatching(NetworkChannel const& channel, SomeIpMessage const& message) const;

    void start(NetworkChannel& channel, SomeIpMessage const& message, ITpListener& listener);

    void stop();

    TpResult
    receive(NetworkChannel const& channel, SomeIpMessage const& message, uint32_t timestamp);

protected:
    explicit TpReceiver(::etl::span<uint8_t> const& buffer) : _buffer(buffer) {}

private:
    ::etl::span<uint8_t> _buffer;
    NetworkChannel* _pChannel = nullptr;

    ITpListener* _pListener = nullptr;
    uint32_t _timestamp     = 0;

    size_t _totalPayloadLength    = 0;
    size_t _receivedPayloadLength = 0;
};

namespace declare
{
template<size_t BufferSize>
class TpReceiver : public ::someip::TpReceiver
{
public:
    TpReceiver() : ::someip::TpReceiver(_buffer), _buffer() {}

private:
    ::etl::array<uint8_t, BufferSize> _buffer;
};
} // namespace declare
} // namespace someip
