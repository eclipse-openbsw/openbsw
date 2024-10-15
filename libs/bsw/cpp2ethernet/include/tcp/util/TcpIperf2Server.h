// Copyright 2025 Accenture.

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
    void sendByte(uint8_t byte);

    bool _locked;
    AbstractSocket& _socket;
    uint32_t _acknowledged;
    uint32_t _counter;
};

} // namespace tcp
