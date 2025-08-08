// Copyright 2025 Accenture.

#pragma once

#include "tcp/socket/ISocketProvidingConnectionListener.h"

#include <platform/estdint.h>

/**
 * contains TCP abstraction layer related code
 * \namespace tcp
 */
namespace tcp
{
/**
 * Abstract base class for server sockets.
 *
 * An AbstractServerSocket is the base class for server sockets that may
 * open a specific port. To accept incoming connections the AbstractServerSocket
 * needs a ISocketProvidingConnectionListener. On a incoming connection request
 * the AbstractServerSocket requests an AbstractSocket object from the
 * ISocketProvidingConnectionListener to bind the connection to via its
 * getSocket() method. In case of success the listeners connectionAccepted()
 * method is called passing the bound AbstractSocket to the application.
 *
 * \see   ISocketProvidingConnectionListener
 */
class AbstractServerSocket
{
public:
    AbstractServerSocket();
    AbstractServerSocket(AbstractServerSocket const&)            = delete;
    AbstractServerSocket& operator=(AbstractServerSocket const&) = delete;

    // [AbstractServerSocket]
    /**
     * constructor taking the AbstractServerSockets port
     * \param   port   port of socket
     * \param   providingListener  ISocketProvidingConnectionListener which
     *          provides the AbstractServerSocket with AbstractSocket objects
     *          and gets notified when a connection is successfully established
     *
     * As long as the ISocketProvidingConnectionListener returns valid pointers
     * to AbstractSocket objects connections are accpeted and delegated to the
     * ISocketProvidingConnectionListener. When NULL is returned, the connection
     * is refused.
     */
    AbstractServerSocket(uint16_t port, ISocketProvidingConnectionListener& providingListener);

    /**
     * \return  port the AbstractServerSocket is listening for
     */
    uint16_t getLocalPort() const { return _port; }

    void setPort(uint16_t const port) { _port = port; }

    /**
     * accepts a connection to the AbstractServerSockets port
     */
    virtual bool accept() = 0;

    /**
     * Binds this server socket to a given netif.
     */
    virtual bool bind(ip::IPAddress const& localIpAddress, uint16_t port) = 0;

    /**
     * closes the accepted port
     */
    virtual void close() = 0;

    virtual bool isClosed() const = 0;

    // [AbstractServerSocket]

    void setSocketProvidingConnectionListener(ISocketProvidingConnectionListener& listener)
    {
        _socketProvidingConnectionListener = &listener;
    }

    ISocketProvidingConnectionListener& getSocketProvidingConnectionListener() const
    {
        return *_socketProvidingConnectionListener;
    }

protected:
    /** AbstractServerSockets port */
    uint16_t _port;
    /** reference to ISocketProvidingConnectionListener */
    ISocketProvidingConnectionListener* _socketProvidingConnectionListener;
};

} // namespace tcp
