// Copyright 2025 Accenture.

#pragma once

#include "tcp/IDataListener.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"

namespace tcp
{
class AbstractSocket;

class BandwidthTestSocket
: public ISocketProvidingConnectionListener
, public IDataListener
{
public:
    explicit BandwidthTestSocket(AbstractSocket& socket);

    AbstractSocket* getSocket(ip::IPAddress const& ipAddr, uint16_t port) override;

    void connectionAccepted(AbstractSocket& socket) override;

    void dataReceived(uint16_t length) override;

    void connectionClosed(ErrorCode status) override;

private:
    bool _locked;
    AbstractSocket& _socket;
};

} // namespace tcp
