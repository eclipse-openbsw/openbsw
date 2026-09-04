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

#include "someip/NetworkChannel.h"
#include "someip/SomeIpMessage.h"

#include <etl/array.h>
#include <etl/span.h>
#include <cstdint>

namespace someip
{
/**
 * Implementation of a TP sender.
 */
class TpSender
{
public:
    enum class TpResult : uint8_t
    {
        TP_OK,
        TP_ERROR
    };

    TpResult send(NetworkChannel& channel, SomeIpMessage const& message);

protected:
    explicit TpSender(::etl::span<uint8_t> const& buffer) : _buffer(buffer) {}

private:
    ::etl::span<uint8_t> _buffer;
};

namespace declare
{
template<size_t BufferSize>
class TpSender : public ::someip::TpSender
{
public:
    TpSender() : ::someip::TpSender(_buffer), _buffer() {}

private:
    ::etl::array<uint8_t, BufferSize> _buffer;
};
} // namespace declare
} // namespace someip
