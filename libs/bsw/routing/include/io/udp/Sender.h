/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <io/IReader.h>
#include <routing/util.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <platform/estdint.h>

namespace io
{
namespace udp
{
class Sender
{
public:
    Sender(::io::IReader& input, ::udp::AbstractDatagramSocket& socket)
    : _input(input), _socket(socket), _socketErrorPdus()
    {}

    void run(size_t const maxNumFrames) { send(maxNumFrames); }

    ::routing::StatCounter::Type socketErrorPdus() const { return _socketErrorPdus; }

private:
    void send(size_t const maxNumFrames)
    {
        size_t count = 0U;
        auto data    = _input.peek();
        while ((count < maxNumFrames) && (!data.empty()))
        {
            if (_socket.send(data) != ::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK)
            {
                ++_socketErrorPdus;
            }
            _input.release();
            count++;
            data = _input.peek();
        }
    }

    ::io::IReader& _input;
    ::udp::AbstractDatagramSocket& _socket;

    mutable ::routing::StatCounter _socketErrorPdus;
};

} // namespace udp
} // namespace io
