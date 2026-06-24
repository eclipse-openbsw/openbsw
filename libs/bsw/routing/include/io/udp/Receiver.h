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

#include <etl/memory.h>
#include <io/IWriter.h>
#include <ip/IPAddress.h>
#include <routing/ErrorHandler.h>
#include <routing/Logger.h>
#include <routing/util.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <platform/estdint.h>

namespace io
{
namespace udp
{
namespace logger = ::util::logger;

class Receiver : private ::udp::IDataListener
{
public:
    Receiver(
        ::udp::AbstractDatagramSocket& socket,
        ::io::IWriter& output,
        ::routing::ErrorHandler const errorHandler,
        ::etl::span<uint8_t const> const remoteIpAddresses = {})
    : _socket(socket)
    , _output(output)
    , _errorHandler(errorHandler)
    , _remoteIpAddresses(remoteIpAddresses)
    , _lastAllocationFailed(false)
    , _invalidIpAddressPdus()
    , _failedMemAllocPdus()
    {
        _socket.setDataListener(this);
    }

    virtual ~Receiver() {}

    ::routing::StatCounter::Type invalidIpAddressPdus() const { return _invalidIpAddressPdus; }

    ::routing::StatCounter::Type failedMemAllocPdus() const { return _failedMemAllocPdus; }

private:
    void dataReceived(
        ::udp::AbstractDatagramSocket& /* socket */,
        ::ip::IPAddress sourceAddress,
        uint16_t /* sourcePort */,
        ::ip::IPAddress /* destinationAddress */,
        uint16_t length) override
    {
        if (!_remoteIpAddresses.empty())
        {
            auto const sourceIpAddress = ::ip::packed(sourceAddress);
            auto remoteIpAddresses     = _remoteIpAddresses;
            bool isValidAddress        = false;
            while (!remoteIpAddresses.empty())
            {
                auto const remoteIpAddress
                    = remoteIpAddresses.take<uint8_t const>(sourceIpAddress.size());
                if (::etl::mem_compare(
                        sourceIpAddress.begin(), sourceIpAddress.end(), remoteIpAddress.begin())
                    == 0)
                {
                    isValidAddress = true;
                    break;
                }
            }

            if (!isValidAddress)
            {
                (void)_socket.read(nullptr, length);

                _errorHandler(
                    ::routing::ErrorHandler::StatusCode::INVALID_REMOTE_IP_ADDRESS, sourceAddress);

                ++_invalidIpAddressPdus;

                return;
            }
        }

        auto const frame = _output.allocate(length);
        if (frame.size() != length)
        {
            (void)_socket.read(nullptr, length);

            if (!_lastAllocationFailed)
            {
                _errorHandler(::routing::ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE);
                _lastAllocationFailed = true;
            }

            ++_failedMemAllocPdus;

            return;
        }
        _lastAllocationFailed = false;

        (void)_socket.read(frame.data(), length);

        _output.commit();
    }

    ::udp::AbstractDatagramSocket& _socket;
    ::io::IWriter& _output;
    ::routing::ErrorHandler const _errorHandler;
    ::etl::span<uint8_t const> const _remoteIpAddresses;
    bool _lastAllocationFailed;

    mutable ::routing::StatCounter _invalidIpAddressPdus;
    mutable ::routing::StatCounter _failedMemAllocPdus;
};

} // namespace udp
} // namespace io
