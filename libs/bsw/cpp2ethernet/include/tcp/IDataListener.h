// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace tcp
{
/**
 * interface for classes that want to receive data from a AbstractSocket
 *
 * \see     AbstractSocket
 */
class IDataListener
{
public:
    IDataListener(IDataListener const&)            = delete;
    IDataListener& operator=(IDataListener const&) = delete;

    /**
     * ErrorCodes used by IDataListener
     */
    enum class ErrorCode : uint8_t
    {
        /** TCP connection was closed, i.e. by a FIN packet */
        ERR_CONNECTION_CLOSED,
        /** TCP connection was reset, i.e. by a RST packet */
        ERR_CONNECTION_RESET,
        /** TCP connection was timed out */
        ERR_CONNECTION_TIMED_OUT,
        /** unknown error */
        ERR_UNKNOWN
    };

    /**
     * default constructor
     */
    IDataListener() = default;

    /**
     * Callback that gets called when data has been received.
     * \param   length   Number of bytes available to read.
     *
     * \note
     * It is not required to read the data immediately from within this callback, the socket
     * will store the data and the next dataReceived callback will have a larger length
     * parameter. However, keep in mind that TCP has a WINDOW_SIZE. If data is not read from
     * the socket and length becomes WINDOW_SIZE, not further dataReceived callback will
     * occur because the sender cannot send any more data as the window size is zero.
     */
    virtual void dataReceived(uint16_t length) = 0;

    /**
     * Callback that gets called when the registered AbstractTCPSocket got closed by peer
     * request.
     * \param   status  ErrorCode indication why connection was closed
     */
    virtual void connectionClosed(ErrorCode status) = 0;
};

} // namespace tcp
