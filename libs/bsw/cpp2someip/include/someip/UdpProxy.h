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

#include "someip/NetworkResource.h"

#include <ip/IPAddress.h>
#include <ip/IPEndpoint.h>
#include <udp/IDataListener.h>
#include <udp/socket/AbstractDatagramSocket.h>

#include <cstdint>

namespace someip
{
/**
 * A UDP proxy for a protocol specific network-resource.
 */
class UdpProxy
: public NetworkResource
, public ::udp::IDataListener
{
public:
    UdpProxy();

    bool isInitialized() const override;

    void setSocket(::udp::AbstractDatagramSocket& socket);
    ::udp::AbstractDatagramSocket* getSocket() const;

    bool isOpen() const override;
    bool isConnected() const override;

    ::etl::expected<uint16_t, PortError> getLocalPort() const override;
    uint8_t getProto() const override;

    bool open(::ip::IPEndpoint const& localEndpoint);

    /**
     * Joins a multicast group.
     * \param multicastIp  multicast IP address
     *
     * \remark This method will fail unless called after open().
     *
     * \return  true if successful
     */
    bool join(::ip::IPAddress const& multicastIp);

    void close() override;

    bool send(::ip::IPEndpoint const& remoteEndpoint, uint32_t length) override;
    bool send(uint32_t length) override;

    /** \see IDataListener */
    void dataReceived(
        ::udp::AbstractDatagramSocket& socket,
        ::ip::IPAddress sourceAddress,
        uint16_t sourcePort,
        ::ip::IPAddress destinationAddress,
        uint16_t length) override;

private:
    /**
     * called when dataReceived() failed
     */
    void flush(uint16_t length);

    ::udp::AbstractDatagramSocket* _pSocket;
};

} // namespace someip
