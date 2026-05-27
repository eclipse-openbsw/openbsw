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

#include "tcp/IDataListener.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"

namespace tcp
{
class AbstractSocket;

class TcpIperf2Server
: public ISocketProvidingConnectionListener
, public IDataListener
{
public:
    explicit TcpIperf2Server(AbstractSocket& socket);

    AbstractSocket* getSocket(ip::IPAddress const& ipAddr, uint16_t port) override;

    void connectionAccepted(AbstractSocket& socket) override;

    void dataReceived(uint16_t length) override;

    void connectionClosed(ErrorCode status) override;

private:
    bool _locked;
    AbstractSocket& _socket;
};

} // namespace tcp
