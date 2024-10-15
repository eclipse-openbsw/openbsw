// Copyright 2025 Accenture.

#pragma once

#include "ip/IPAddress.h"
#include "udp/DatagramPacket.h"
#include "udp/IDataListener.h"
#include "udp/IDataSentListener.h"

#include <etl/span.h>

#include <platform/estdint.h>

namespace udp
{
/**
 * Interface for all udp sockets.
 */
class AbstractDatagramSocket
{
public:
    // [AbstractDatagramSocket]
    enum
    {
        INVALID_PORT = 0xFFFF
    };

    /**
     * all ErrorCodes used by socket
     * \enum  ErrorCode
     */
    enum class ErrorCode : uint8_t
    {
        /** everything ok */
        UDP_SOCKET_OK,
        /** an error occurred */
        UDP_SOCKET_NOT_OK,
        /** no IDataListener registerd */
        UDP_SOCKET_NO_DATA_LISTENER
    };

    AbstractDatagramSocket();
    AbstractDatagramSocket(AbstractDatagramSocket const&)            = delete;
    AbstractDatagramSocket& operator=(AbstractDatagramSocket const&) = delete;

    /**
     * Binds this UDPSocket to a specific port & local address
     * \param port  local port
     * \return  status of method
     *          - UDP_SOCKET_OK when everything was fine
     *          - UDP_SOCKET_NOT_OK when something went wrong
     */
    virtual ErrorCode bind(ip::IPAddress const* pIpAddress, uint16_t port) = 0;

    virtual ErrorCode join(ip::IPAddress const& groupAddr) = 0;

    /**
     * Returns the binding state of the socket.
     * \return  binding state of the socket
     *          - false: not bound
     *          - true: bound
     */
    virtual bool isBound() const = 0;

    virtual void close() = 0;

    /**
     * Returns whether the socket is closed or not.
     * \return  state of the socket
     *          - false: not closed
     *          - true: closed
     */
    virtual bool isClosed() const = 0;

    /**
     * Connects the socket to a remote address for this socket.
     * \param   address  remote IPAddress
     * \param   port     remote port
     * \return  status of method
     *          - UDP_SOCKET_OK when everything was fine
     *          - UDP_SOCKET_NOT_OK when something went wrong
     */
    virtual ErrorCode
    connect(ip::IPAddress const& address, uint16_t port, ip::IPAddress* pLocalAddress)
        = 0;

    /**
     * Disconnects the socket
     */
    virtual void disconnect() = 0;

    /**
     * Returns the connection state of the socket.
     * \return  connection state of the socket
     *          - false: not connected
     *          - true: connected
     */
    virtual bool isConnected() const = 0;

    /**
     * reads a given number of bytes from the socket
     * \param  buffer   buffer to receive data to. If 0L is passed the n
     * bytes should be skipped, i.e. the input stream has to advance by
     * n bytes.
     * \param   n   max number of bytes to receive
     * \return  number of bytes really read
     *          - 0: an error occurred reading from the socket
     *          - else: bytes read
     */
    virtual size_t read(uint8_t* buffer, size_t n) = 0;

    /**
     * sends an amount of data
     * \param  data   data to send
     * \return  status of transmission
     *          - UDP_SOCKET_OK when data was sent to UDP stack
     *          - UDP_SOCKET_NOT_OK when socket has not been opened
     * \note
     * sending the data may be asynchronous. for this reason the
     * IDataSendNotificationListener has a appropriate callback.
     */
    virtual ErrorCode send(::etl::span<uint8_t const> const& data) = 0;

    /**
     * Sends a DatagramPacket.
     * \return  status of transmission
     *          - UDP_SOCKET_OK when data was sent to UDP stack
     *          - UDP_SOCKET_NOT_OK when socket has not been opened
     */
    virtual ErrorCode send(DatagramPacket const& packet) = 0;

    /**
     * sets the listener to this socket instance
     * \param  pListener  IDataListener to attach
     */
    void setDataListener(IDataListener* pListener);

    void setDataSentListener(IDataSentListener* pListener);

    /**
     * \return  Local IPAddress, 0L if this operation is not allowed
     */
    virtual ip::IPAddress const* getIPAddress() const = 0;

    virtual ip::IPAddress const* getLocalIPAddress() const = 0;

    /**
     * Returns remote port if connected, INVALID_PORT otherwise
     */
    virtual uint16_t getPort() const = 0;

    virtual uint16_t getLocalPort() const = 0;

    // [AbstractDatagramSocket]
protected:
    IDataListener* _dataListener;
    IDataSentListener* _dataSentListener;
};

/*
 * inline methods
 */
inline void AbstractDatagramSocket::setDataListener(IDataListener* const pListener)
{
    _dataListener = pListener;
}

inline void AbstractDatagramSocket::setDataSentListener(IDataSentListener* const pListener)
{
    _dataSentListener = pListener;
}

} // namespace udp
