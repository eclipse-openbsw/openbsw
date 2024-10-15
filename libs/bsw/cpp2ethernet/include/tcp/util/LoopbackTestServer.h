// Copyright 2025 Accenture.

#pragma once

#include "tcp/IDataListener.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"

#include <etl/array.h>

namespace tcp
{
class AbstractSocket;

class LoopbackTestServer
: public ISocketProvidingConnectionListener
, public IDataListener
{
public:
    explicit LoopbackTestServer(AbstractSocket& socket);

    AbstractSocket* getSocket(ip::IPAddress const& ipAddr, uint16_t port) override;

    void connectionAccepted(AbstractSocket& socket) override;

    void dataReceived(uint16_t length) override;

    void connectionClosed(ErrorCode status) override;

private:
    void sendByte(uint8_t byte);
    void sendData(::etl::span<uint8_t> data);

    bool _locked;
    AbstractSocket& _socket;
    // uint32_t _acknowledged;
    // uint32_t _counter;
    static constexpr size_t TCP_RECV_SIZE{1460U};
    ::etl::array<uint8_t, TCP_RECV_SIZE> _receiveData;
};

} // namespace tcp
