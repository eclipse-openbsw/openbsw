// Copyright 2025 Accenture.

#pragma once

#include <etl/delegate.h>
#include <etl/span.h>
#include <ip/IPAddress.h>

#include <cstdint>

namespace tcp
{
class IDataListener;
class IDataSendNotificationListener;

/**
 * Abstract base class for all sockets.
 *
 * \section Overview
 * The AbstractSocket is designed to be a socket abstraction to
 * a TCP stack.
 * The socket class can be used to connect to a remote system. It is also
 * returned to an ISocketConnectionListener by AbstractServerSocket
 * (using its ISocketProvider) once a connection from a remote system
 * is accepted. An active socket is used to receive and transmit data
 * from and to a remote system.
 *
 * \section Receive
 * The application that wants to receive data from a socket has to register
 * through the setDataListener method as an IDataListener.
 * After that the application is notified through its dataReceived
 * method.
 *
 * \section Send
 * Sending data through a socket is usually asynchronous. Because of that
 * it is possible to register an IDataSendNotificationListener who is
 * notified when the data passed to the send method is written to the
 * TCP-stack.
 * <b>This is not the time when the ACK package from the remote
 * system, is received!</b>
 *
 * \see     AbstractServerSocket
 * \see     ISocketProvidingConnectionListener
 * \see     IDataListener
 * \see     IDataSendNotificationListener
 */
class AbstractSocket
{
public:
    // [AbstractSocket]
    /**
     * all ErrorCodes used by socket
     * \enum   ErrorCode
     */
    enum class ErrorCode : uint8_t
    {
        /** everything ok */
        SOCKET_ERR_OK,
        /** an error occurred */
        SOCKET_ERR_NOT_OK,
        /** socket not open to send */
        SOCKET_ERR_NOT_OPEN,
        /** no more outgoing buffer available */
        SOCKET_ERR_NO_MORE_BUFFER,
        /** socket is full, flush before continuing */
        SOCKET_FLUSH
    };

    using ConnectedDelegate = ::etl::delegate<void(ErrorCode)>;

    /**
     * constructor
     * \post    fpDataListener == 0L
     * \post    fpSendNotificationListener == 0L
     */
    AbstractSocket();

    AbstractSocket(AbstractSocket const&)            = delete;
    AbstractSocket& operator=(AbstractSocket const&) = delete;

    /**
     * binds to a local address and port
     * \param   ipAddr  local ip address
     * \param   port    local port
     */
    virtual ErrorCode bind(ip::IPAddress const& ipAddr, uint16_t port) = 0;

    /**
     * connects to a remote host
     * \param   ipAddr  remote ip address
     * \param   port    remote port
     */
    virtual ErrorCode
    connect(ip::IPAddress const& ipAddr, uint16_t port, ConnectedDelegate delegate)
        = 0;

    /**
     * closes the socket by sending a FIN packet
     */
    virtual ErrorCode close() = 0;

    /**
     * aborts the socket by sending a RST packet
     */
    virtual void abort() = 0;

    /**
     * flushes enqueued data
     */
    virtual ErrorCode flush() = 0;

    /**
     * discards any received data without ack
     */
    virtual void discardData() = 0;

    /**
     * \return  number of bytes available in outgoing buffer
     */
    virtual size_t available() = 0;

    /**
     * reads a single byte
     * \param   byte   byte to read
     * \return  number of bytes read
     *          - 0: an error occurred reading from the socket
     *          - 1: byte read
     */
    virtual uint8_t read(uint8_t& byte) = 0;

    /**
     * reads a given number of bytes from the socket
     * \param   buffer  buffer to receive data to. If 0L is passed the n
     * bytes should be skipped, i.e. the input stream has to advance by
     * n bytes.
     * \param   n    number of bytes to receive
     * \return  number of bytes really read
     *          - 0: an error occurred reading from the socket
     *          - else: bytes read
     */
    virtual size_t read(uint8_t* buffer, size_t n) = 0;

    /**
     * sends an amount of data
     * \param   data  data to send
     * \return  status of transmission
     *          - SOCKET_OK when data was sent to TCP stack
     *          - SOCKET_CLOSED when socket has not been opened
     * \note
     * Sending the data may be asynchronous. For this reason the
     * IDataSendNotificationListener has an appropriate callback.
     */
    virtual ErrorCode send(::etl::span<uint8_t const> const& data) = 0;

    /**
     * sets the listener to this socket instance
     * \param  pListener  IDataListener to attach
     */
    void setDataListener(IDataListener* pListener);

    IDataListener* getDataListener() const;

    /**
     * sets the IDataSendNotificationListener of this socket
     * \param  pListener   listener that gets notified when data has been
     * copied completely to TCP stack
     */
    void setSendNotificationListener(IDataSendNotificationListener* pListener);

    IDataSendNotificationListener* getSendNotificationListener() const;

    virtual ip::IPAddress getRemoteIPAddress() const = 0;

    virtual ip::IPAddress getLocalIPAddress() const = 0;

    virtual uint16_t getRemotePort() const = 0;

    virtual uint16_t getLocalPort() const = 0;

    virtual bool isClosed() const = 0;

    virtual bool isEstablished() const = 0;

    /**
     * Permanently disables NAGLE algorithm for this socket.
     */
    virtual void disableNagleAlgorithm() = 0;

    /**
     * Enables TCP keepalive for this socket.
     * \param idle the length of time to keep an idle TCP connection active
     * \param interval the interval between packets sent to validate the TCP connection
     * \param probes the number of keepalive probes to be sent before terminating the connection
     */
    virtual void enableKeepAlive(uint32_t idle, uint32_t interval, uint32_t probes) = 0;

    /**
     * Disables TCP keepalive for this socket.
     */
    virtual void disableKeepAlive() = 0;
    // [AbstractSocket]

protected:
    IDataListener* _dataListener;
    IDataSendNotificationListener* _sendNotificationListener;
};

/*
 * inline methods
 */
inline void AbstractSocket::setDataListener(IDataListener* const pListener)
{
    _dataListener = pListener;
}

inline void
AbstractSocket::setSendNotificationListener(IDataSendNotificationListener* const pListener)
{
    _sendNotificationListener = pListener;
}

inline IDataListener* AbstractSocket::getDataListener() const { return _dataListener; }

inline IDataSendNotificationListener* AbstractSocket::getSendNotificationListener() const
{
    return _sendNotificationListener;
}

} // namespace tcp
