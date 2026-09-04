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

#include "tcp/socket/AbstractServerSocket.h"
#include "tcp/socket/AbstractSocket.h"

class SocketDummy : public ::tcp::AbstractSocket
{
public:
    ::ip::IPAddress getRemoteIPAddress() const override { return ::ip::IPAddress(); }

    ::ip::IPAddress getLocalIPAddress() const override { return ::ip::IPAddress(); }

    ErrorCode bind(::ip::IPAddress const&, uint16_t) override
    {
        return ErrorCode::SOCKET_ERR_NOT_OK;
    }

    ErrorCode close() override { return ErrorCode::SOCKET_ERR_NOT_OK; }

    ErrorCode connect(::ip::IPAddress const&, uint16_t, ConnectedDelegate) override
    {
        return ErrorCode::SOCKET_ERR_NOT_OK;
    }

    ErrorCode flush() override { return ErrorCode::SOCKET_ERR_NOT_OK; }

    void discardData() override {}

    ErrorCode send(::etl::span<uint8_t const> const&) override
    {
        return ErrorCode::SOCKET_ERR_NOT_OK;
    }

    bool isClosed() const override { return false; }

    bool isEstablished() const override { return false; }

    size_t available() override { return size_t(); }

    size_t read(uint8_t*, size_t) override { return size_t(); }

    uint16_t getLocalPort() const override { return uint16_t(); }

    uint16_t getRemotePort() const override { return uint16_t(); }

    uint8_t read(uint8_t&) override { return uint8_t(); }

    void abort() override {}

    void disableNagleAlgorithm() override {}

    void enableKeepAlive(uint32_t const, uint32_t const, uint32_t const) override {}

    void disableKeepAlive() override {}
};

class ServerSocketDummy : public ::tcp::AbstractServerSocket
{
public:
    bool accept() override { return false; }

    bool bind(::ip::IPAddress const&, uint16_t) override { return false; }

    void close() override {}

    bool isClosed() const override { return true; }
};
