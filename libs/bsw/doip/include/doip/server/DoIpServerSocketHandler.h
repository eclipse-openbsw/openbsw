// Copyright 2025 Accenture.

#pragma once

#include "DoIpServerLogger.h"
#include "doip/common/DoIpConstants.h"
#include "doip/server/IDoIpServerSocketHandler.h"
#include "doip/server/IDoIpServerSocketHandlerListener.h"

#include <etl/pool.h>
#include <etl/vector.h>
#include <ip/NetworkInterfaceConfig.h>
#include <tcp/socket/ISocketProvidingConnectionListener.h>
#include <util/logger/Logger.h>

namespace doip
{
namespace declare
{
/**
 * Socket handler class that allocates both TCP server sockets and connection sockets.
 * \tparam ServerSocket server socket class
 * \tparam Socket socket class
 * \tparam NUM_SERVER_SOCKETS number of server sockets to allocate
 * \tparam NUM_PLAIN_SOCKETS number of connection sockets to allocate. One of these is required for
 * each DoIP connection (non-TLS and TLS) and thus limits the maximum number of DoIP connections
 * independent of the connection type.
 * \tparam NUM_TLS_SOCKETS number of connection sockets (TLS only) to allocate. This limits the
 * maximum number of TLS connections.
 */
template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS = 0U>
class DoIpServerSocketHandler : public IDoIpServerSocketHandler
{
    static_assert(
        NUM_PLAIN_SOCKETS >= NUM_TLS_SOCKETS,
        "Number of plain sockets must be greater or equal to number of TLS sockets.");

public:
    /**
     * Constructor.
     * \param networkInterfaceConfigRegistry reference to network interface registry
     */
    explicit DoIpServerSocketHandler(
        ::ip::NetworkInterfaceConfigRegistry& networkInterfaceConfigRegistry);

    /**
     * Add a server socket that will automatically be bound when configured.
     * \param serverSocketId identifier to be used when reporting events to the listener
     * \param networkInterfaceConfigKey key that statically identifies the interface address to bind
     * to \return reference to the server socket
     */
    ServerSocket& addServerSocket(
        uint8_t serverSocketId, ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey);

    /** \see    ::doip::IDoIpServerSocketHandler::start() */
    void start(IDoIpServerSocketHandlerListener& listener) override;
    /** \see    ::doip::IDoIpServerSocketHandler::stop() */
    void stop() override;
    /** \see    ::doip::IDoIpServerSocketHandler::acquireSocket() */
    ::tcp::AbstractSocket* acquireSocket() override;
    /** \see    ::doip::IDoIpServerSocketHandler::releaseSocket() */
    void
    releaseSocket(::tcp::AbstractSocket& socket, DoIpTcpConnection::ConnectionType type) override;

private:
    static constexpr size_t RELEASE_TLS_SOCKETS_INTERVAL_MS = 200;

    class SocketHandler : public ::tcp::ISocketProvidingConnectionListener
    {
    public:
        SocketHandler(
            DoIpServerSocketHandler& parent,
            uint8_t serverSocketId,
            ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
            DoIpTcpConnection::ConnectionType type);

        ServerSocket& getServerSocket();
        ::ip::NetworkInterfaceConfigKey const& getNetworkInterfaceConfigKey() const;

        void updateSocket(::ip::NetworkInterfaceConfig const& config);

        ::tcp::AbstractSocket* getSocket(::ip::IPAddress const& ipAddr, uint16_t port) override;
        void connectionAccepted(::tcp::AbstractSocket& socket) override;

    private:
        ServerSocket _serverSocket;
        ::ip::NetworkInterfaceConfigKey _networkInterfaceConfigKey;
        DoIpServerSocketHandler& _parent;
        uint8_t _serverSocketId;
        DoIpTcpConnection::ConnectionType _type;
    };

    using SocketHandlerVector = ::etl::ivector<SocketHandler>;

    void
    configChanged(::ip::NetworkInterfaceConfigKey key, ::ip::NetworkInterfaceConfig const& config);

    ::etl::vector<SocketHandler, NUM_SERVER_SOCKETS> _socketHandlers;
    ::etl::pool<Socket, NUM_PLAIN_SOCKETS> _sockets;
    ::ip::ConfigChangedSlotType _configChangedSlot;
    ::ip::NetworkInterfaceConfigRegistry& _networkInterfaceConfigRegistry;
    ::doip::IDoIpServerSocketHandlerListener* _listener;
};

/**
 * Inline implementations.
 */
template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::DoIpServerSocketHandler(::ip::NetworkInterfaceConfigRegistry&
                                                  networkInterfaceConfigRegistry)
: IDoIpServerSocketHandler()
, _configChangedSlot(
      ::ip::ConfigChangedSlotType::
          create<DoIpServerSocketHandler, &DoIpServerSocketHandler::configChanged>(*this))
, _networkInterfaceConfigRegistry(networkInterfaceConfigRegistry)
, _listener(nullptr)
{}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
ServerSocket& DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::
    addServerSocket(
        uint8_t const serverSocketId,
        ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey)
{
    SocketHandler& socketHandler = _socketHandlers.emplace_back(
        *this, serverSocketId, networkInterfaceConfigKey, DoIpTcpConnection::ConnectionType::PLAIN);
    return socketHandler.getServerSocket();
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
void DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::start(IDoIpServerSocketHandlerListener& listener)
{
    _listener = &listener;
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.updateSocket(_networkInterfaceConfigRegistry.getConfig(
            socketHandler.getNetworkInterfaceConfigKey()));
    }

    _networkInterfaceConfigRegistry.connect(_configChangedSlot);
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
void DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::stop()
{
    _networkInterfaceConfigRegistry.disconnect(_configChangedSlot);
    for (auto& socketHandler : _socketHandlers)
    {
        socketHandler.updateSocket(::ip::NetworkInterfaceConfig());
    }
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
void DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::
    releaseSocket(::tcp::AbstractSocket& socket, DoIpTcpConnection::ConnectionType type)
{
    if (type == DoIpTcpConnection::ConnectionType::PLAIN)
    {
        // Only sockets of the derived class are referenced here
        _sockets.destroy(static_cast<Socket*>(&socket));
    }
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
void DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::
    configChanged(::ip::NetworkInterfaceConfigKey key, ::ip::NetworkInterfaceConfig const& config)
{
    for (auto& socketHandler : _socketHandlers)
    {
        if (socketHandler.getNetworkInterfaceConfigKey() == key)
        {
            socketHandler.updateSocket(config);
        }
    }
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
::tcp::AbstractSocket* DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::acquireSocket()
{
    if (!_sockets.full())
    {
        return _sockets.create();
    }
    return nullptr;
}

/**
 * SocketHandler implementation.
 */
template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::SocketHandler::
    SocketHandler(
        DoIpServerSocketHandler& parent,
        uint8_t const serverSocketId,
        ::ip::NetworkInterfaceConfigKey const& networkInterfaceConfigKey,
        DoIpTcpConnection::ConnectionType const type)
: ::tcp::ISocketProvidingConnectionListener()
, _serverSocket{}
, _networkInterfaceConfigKey(networkInterfaceConfigKey)
, _parent(parent)
, _serverSocketId(serverSocketId)
, _type(type)
{}

// sockets are managed from the outside
template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
inline ServerSocket& DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::SocketHandler::getServerSocket()
{
    return _serverSocket;
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
inline ::ip::NetworkInterfaceConfigKey const& DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::SocketHandler::getNetworkInterfaceConfigKey() const
{
    return _networkInterfaceConfigKey;
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
::tcp::AbstractSocket* DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::SocketHandler::getSocket(::ip::IPAddress const& ipAddr, uint16_t port)
{
    if (_parent._listener->filterConnection(_serverSocketId, ::ip::IPEndpoint(ipAddr, port)))
    {
        return _parent.acquireSocket();
    }
    return nullptr;
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
void DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::SocketHandler::connectionAccepted(::tcp::AbstractSocket& socket)
{
    tcp::AbstractSocket* abstractSocket = nullptr;
    abstractSocket                      = &socket;
    _parent._listener->connectionAccepted(_serverSocketId, *abstractSocket, _type);
}

template<
    class ServerSocket,
    class Socket,
    size_t NUM_SERVER_SOCKETS,
    size_t NUM_PLAIN_SOCKETS,
    size_t NUM_TLS_SOCKETS>
void DoIpServerSocketHandler<
    ServerSocket,
    Socket,
    NUM_SERVER_SOCKETS,
    NUM_PLAIN_SOCKETS,
    NUM_TLS_SOCKETS>::SocketHandler::updateSocket(::ip::NetworkInterfaceConfig const& config)
{
    if (!_serverSocket.isClosed())
    {
        _serverSocket.close();
    }
    if (config.isValid())
    {
        _serverSocket.setSocketProvidingConnectionListener(*this);
        auto const port = _type == DoIpTcpConnection::ConnectionType::TLS
                              ? DoIpConstants::Ports::TCP_DATA_TLS
                              : DoIpConstants::Ports::TCP_DATA;
        if (!_serverSocket.bind(config.ipAddress(), port))
        {
            ::util::logger::Logger::warn(
                ::util::logger::DOIP, "updateSocket(): bind() on port %u failed", port);
            return;
        }
        if (!_serverSocket.accept())
        {
            ::util::logger::Logger::warn(
                ::util::logger::DOIP, "updateSocket(): accept() on port %u failed", port);
            return;
        }
        _parent._listener->serverSocketBound(
            _serverSocketId, ::ip::IPEndpoint(config.ipAddress(), port), _type);
    }
}

} // namespace declare
} // namespace doip
