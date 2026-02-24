// Copyright 2025 Accenture.

#include "doip/common/DoIpTcpConnection.h"

#include "doip/common/DoIpCommonLogger.h"
#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpLock.h"

#include <async/Async.h>

#include <estd/variant.h>

#include <algorithm>

namespace doip
{
using ::estd::slice;
using ::tcp::AbstractSocket;
using ::util::logger::DOIP_COMMON;
using ::util::logger::Logger;

DoIpTcpConnection::DoIpTcpConnection(
    ::async::ContextType const context, AbstractSocket& socket, slice<uint8_t> const writeBuffer)
: IDoIpTcpConnection()
, IDataListener()
, IDataSendNotificationListener()
, _socket(socket)
, _currentReadBuffer()
, _writeBuffer(writeBuffer)
, _handler(nullptr)
, _payloadReceivedCallback()
, _payloadDiscardedCallback()
, _detachCallback()
, _pendingSendJobs()
, _sentJobs()
, _headerBuffer()
, _readPayloadLength(0U)
, _receivedBufferLength(0U)
, _availableReadDataLength(0U)
, _sentDataLength(0U)
, _pendingSendDataLength(0U)
, _sentJobTotalLength(0U)
, _sendTimeout()
, _context(context)
, _connectionState(ConnectionState::INIT)
, _readState(ReadState::HEADER)
, _closeMode(CloseMode::CLOSE)
, _sendBufferIndex(0U)
, _suspendCallCounter(0U)
, _recurseRead(false)
, _recurseWrite(false)
{}

void DoIpTcpConnection::init(IDoIpConnectionHandler& handler)
{
    if (_connectionState != ConnectionState::ACTIVE)
    {
        Logger::debug(
            DOIP_COMMON,
            "TCPConnection(%p, %d, %d)::init(%p)",
            this,
            _connectionState,
            _closeMode,
            &handler);
        if (_socket.isEstablished())
        {
            _connectionState = ConnectionState::ACTIVE;
            _handler         = &handler;
            _socket.setDataListener(this);
            _socket.setSendNotificationListener(this);
            _availableReadDataLength = 0U;
            _readPayloadLength       = 0U;
            _receivedBufferLength    = 0U;
            _sentDataLength          = 0U;
            _pendingSendDataLength   = 0U;
            _sentJobTotalLength      = 0U;
            _readState               = ReadState::HEADER;
            setReadBuffer(_headerBuffer);
            _recurseRead     = false;
            _recurseWrite    = false;
            _sendBufferIndex = 0U;
        }
        else
        {
            Logger::warn(
                DOIP_COMMON,
                "TCPConnection(%p, %d, %d)::init(%p): socket not established",
                this,
                _connectionState,
                _closeMode,
                &handler);
            handler.connectionClosed(true);
        }
    }
}

::ip::IPEndpoint DoIpTcpConnection::getLocalEndpoint() const
{
    return ::ip::IPEndpoint(_socket.getLocalIPAddress(), _socket.getLocalPort());
}

::ip::IPEndpoint DoIpTcpConnection::getRemoteEndpoint() const
{
    return ::ip::IPEndpoint(_socket.getRemoteIPAddress(), _socket.getRemotePort());
}

bool DoIpTcpConnection::receivePayload(
    slice<uint8_t> const payload, PayloadReceivedCallbackType const payloadReceivedCallback)
{
    if ((_readState == ReadState::PAYLOAD) && (_currentReadBuffer.size() == 0U)
        && (payload.size() <= _readPayloadLength) && payloadReceivedCallback)
    {
        setReadBuffer(payload);
        _payloadReceivedCallback = payloadReceivedCallback;
        tryReceive();
        return true;
    }

    return false;
}

void DoIpTcpConnection::endReceiveMessage(
    PayloadDiscardedCallbackType const payloadDiscardedCallback)
{
    if (_readState == ReadState::PAYLOAD)
    {
        if (_readPayloadLength > 0U)
        {
            _readState = ReadState::DISCARD;
            setReadBuffer(::estd::slice<uint8_t>::from_pointer(nullptr, _readPayloadLength));
            _payloadReceivedCallback  = PayloadReceivedCallbackType();
            _payloadDiscardedCallback = payloadDiscardedCallback;
            tryReceive();
        }
        else
        {
            _readState = ReadState::HEADER;
            setReadBuffer(_headerBuffer);
            payloadDiscardedCallback();
        }
    }
    else
    {
        payloadDiscardedCallback();
    }
}

bool DoIpTcpConnection::sendMessage(IDoIpSendJob& sendJob)
{
    if (_connectionState == ConnectionState::ACTIVE)
    {
        uint8_t suspendCounter = 0U;
        {
            // RAII usage
            DoIpLock const lock;
            _pendingSendJobs.push_back(sendJob);
            suspendCounter = _suspendCallCounter;
        }
        if (suspendCounter == 0U)
        {
            // Use `schedule` in order to make it cancelable.
            (void)::async::schedule(
                _context, *this, _sendTimeout, 0, ::async::TimeUnit::MILLISECONDS);
        }
        return true;
    }

    return false;
}

void DoIpTcpConnection::close() { closeConnection(ConnectionState::INACTIVE, false, true); }

void DoIpTcpConnection::setCloseMode(CloseMode closeMode) { _closeMode = closeMode; }

void DoIpTcpConnection::detach(DetachCallbackType const detachCallback)
{
    if (_connectionState == ConnectionState::ACTIVE)
    {
        _detachCallback = detachCallback;
        if (_sentJobs.empty())
        {
            closeConnection(ConnectionState::INACTIVE, false, false);
        }
    }
    else
    {
        detachCallback();
    }
}

void DoIpTcpConnection::dataReceived(uint16_t const length)
{
    _availableReadDataLength = length;
    tryReceive();
}

void DoIpTcpConnection::connectionClosed(ErrorCode const status)
{
    Logger::warn(
        DOIP_COMMON,
        "TCPConnection(%p, %d, %d)::connectionClosed(0x%x) by TCP stack",
        this,
        _connectionState,
        _closeMode,
        status);
    closeConnection(
        ConnectionState::INACTIVE,
        status != IDataListener::ErrorCode::ERR_CONNECTION_TIMED_OUT,
        false);
}

void DoIpTcpConnection::fireMessageIfNotSuspended()
{
    uint8_t suspendedCounter = 0U;
    {
        // RAII usage
        DoIpLock const lock;
        suspendedCounter = _suspendCallCounter;
    }
    if (suspendedCounter == 0U)
    {
        // Use `schedule` in order to make it cancelable.
        (void)::async::schedule(_context, *this, _sendTimeout, 0, ::async::TimeUnit::MILLISECONDS);
    }
}

void DoIpTcpConnection::dataSent(uint16_t const length, SendResult const result)
{
    if (result == IDataSendNotificationListener::SendResult::DATA_SENT)
    {
        _sentDataLength += length;
        if (!_recurseWrite)
        {
            handleDataSent();
            fireMessageIfNotSuspended();
        }
    }
    else if (_pendingSendDataLength > 0U)
    {
        if (length < _pendingSendDataLength)
        {
            _pendingSendDataLength -= length;
        }
        else
        {
            _pendingSendDataLength = 0U;
            fireMessageIfNotSuspended();
        }
    }
    else
    {
        // nothing to do!
    }
}

bool DoIpTcpConnection::processCurrentSendBuffer(IDoIpSendJob& sendJob)
{
    slice<uint8_t const> const sendBuffer = sendJob.getSendBuffer(_writeBuffer, _sendBufferIndex);
    if (sendBuffer.size() > 0U)
    {
        _recurseWrite                          = true;
        _pendingSendDataLength                 = sendBuffer.size();
        AbstractSocket::ErrorCode const result = _socket.send(sendBuffer);
        _recurseWrite                          = false;
        if (result == AbstractSocket::ErrorCode::SOCKET_ERR_NO_MORE_BUFFER)
        {
            (void)_socket.flush();
        }
        else
        {
            _pendingSendDataLength = 0U;
            if (result != AbstractSocket::ErrorCode::SOCKET_ERR_OK)
            {
                (void)_socket.flush();
                (void)::async::schedule(
                    _context, *this, _sendTimeout, 1, ::async::TimeUnit::MILLISECONDS);
                return false;
            }
        }
    }
    ++_sendBufferIndex;
    return true;
}

void DoIpTcpConnection::selectNextSendJob(IDoIpSendJob& currentSendJob)
{
    _sendBufferIndex = 0U;
    {
        // RAII usage
        DoIpLock const lock;
        _pendingSendJobs.pop_front();
        _sentJobs.push_back(currentSendJob);
    }
    if (_pendingSendJobs.empty())
    {
        _recurseWrite = true;
        (void)_socket.flush();
        _recurseWrite = false;
    }
    handleDataSent();
}

void DoIpTcpConnection::execute()
{
    {
        // RAII usage
        DoIpLock const lock;
        if (_suspendCallCounter != 0U)
        {
            return;
        }
    }
    while ((_connectionState == ConnectionState::ACTIVE) && (!_pendingSendJobs.empty())
           && (_pendingSendDataLength == 0U))
    {
        IDoIpSendJob& sendJob = _pendingSendJobs.front();
        if (_sendBufferIndex < sendJob.getSendBufferCount())
        {
            if (!processCurrentSendBuffer(sendJob))
            {
                return;
            }
        }
        else
        {
            selectNextSendJob(sendJob);
        }
    }
}

void DoIpTcpConnection::processNextReadChunk(size_t const bytesRead)
{
    _receivedBufferLength += bytesRead;
    if (_receivedBufferLength == _currentReadBuffer.size())
    {
        // The current read buffer has been completely processed (i. e. read or discarded)
        slice<uint8_t> currentReadBuffer;
        ::std::swap(_currentReadBuffer, currentReadBuffer);
        _recurseRead = true;
        if (_readState == ReadState::HEADER)
        {
            auto headerBuffer                     = currentReadBuffer.reinterpret_as<DoIpHeader>();
            DoIpHeader const& header              = headerBuffer[0];
            _readState                            = ReadState::PAYLOAD;
            _readPayloadLength                    = header.payloadLength;
            auto const headerReceivedContinuation = _handler->headerReceived(header);
            if (::estd::holds_alternative<IDoIpConnection::PayloadDiscardedCallbackType>(
                    headerReceivedContinuation))
            {
                // handler not responsible, but notify it for the discarded payload
                _readState = ReadState::DISCARD;
                setReadBuffer(::estd::slice<uint8_t>::from_pointer(nullptr, _readPayloadLength));
                _payloadReceivedCallback = PayloadReceivedCallbackType();
                _payloadDiscardedCallback
                    = ::estd::get<IDoIpConnection::PayloadDiscardedCallbackType>(
                        headerReceivedContinuation);
            }
        }
        else if (_readState == ReadState::PAYLOAD)
        {
            _readPayloadLength -= currentReadBuffer.size();
            _payloadReceivedCallback(currentReadBuffer);
        }
        else
        {
            // this is the ReadState::DISCARD, so we are done discarding.
            _readState = ReadState::HEADER;
            setReadBuffer(_headerBuffer);
            // Finally call the discard callback only if set
            if (_payloadDiscardedCallback)
            {
                _payloadDiscardedCallback();
            }
        }
        _recurseRead = false;
    }
    _availableReadDataLength -= bytesRead;
}

void DoIpTcpConnection::tryReceive()
{
    while ((!_recurseRead) && (_connectionState == ConnectionState::ACTIVE)
           && (_currentReadBuffer.size() > 0U) && (_availableReadDataLength > 0U))
    {
        size_t const bytesToRead = ::std::min(
            _availableReadDataLength, _currentReadBuffer.size() - _receivedBufferLength);
        // in ReadState::DISCARD, _currentReadBuffer has nullptr as pointer
        size_t const bytesRead = _socket.read(
            (_readState != ReadState::DISCARD)
                ? _currentReadBuffer.offset(_receivedBufferLength).data()
                : nullptr,
            bytesToRead);
        if (bytesRead == bytesToRead)
        {
            processNextReadChunk(bytesRead);
        }
        else
        {
            closeConnection(ConnectionState::ERROR, false, true);
            break;
        }
    }
}

void DoIpTcpConnection::handleDataSent()
{
    while ((_sentDataLength > 0U) && (!_sentJobs.empty()))
    {
        IDoIpSendJob& sendJob = _sentJobs.front();
        if (_sentJobTotalLength == 0U)
        {
            _sentJobTotalLength = sendJob.getTotalLength();
        }
        if (_sentDataLength >= _sentJobTotalLength)
        {
            {
                // RAII usage
                DoIpLock const lock;
                _sentJobs.pop_front();
            }
            _sentDataLength -= _sentJobTotalLength;
            _sentJobTotalLength = 0U;
            sendJob.release(true);
        }
        else
        {
            _sentJobTotalLength -= _sentDataLength;
            _sentDataLength = 0U;
        }
    }
    if (_detachCallback && _sentJobs.empty())
    {
        closeConnection(ConnectionState::INACTIVE, false, false);
    }
}

void DoIpTcpConnection::closeConnection(
    ConnectionState const connectionState, bool const closedByRemotePeer, bool const closeSocket)
{
    Logger::debug(
        DOIP_COMMON,
        "TCPConnection(%p, %d, %d)::closeConnection(0x%x, %d, %d)",
        this,
        _connectionState,
        _closeMode,
        connectionState,
        closedByRemotePeer,
        closeSocket);
    if (_connectionState == ConnectionState::ACTIVE)
    {
        _connectionState                      = connectionState;
        IDoIpConnectionHandler* const handler = _handler;
        _handler                              = nullptr;
        releaseSendJobs(_sentJobs);
        releaseSendJobs(_pendingSendJobs);
        setReadBuffer(slice<uint8_t>());
        if (_detachCallback.has_value())
        {
            _detachCallback();
        }
        if (closeSocket)
        {
            if (_closeMode == CloseMode::CLOSE)
            {
                (void)_socket.close();
            }
            else
            {
                _socket.abort();
            }
        }
        // This is an optimization only, preventing spam of the `execute` method.
        (void)_sendTimeout.cancel();
        handler->connectionClosed(closedByRemotePeer);
    }
}

void DoIpTcpConnection::releaseSendJobs(::estd::forward_list<IDoIpSendJob>& sendJobs)
{
    ::estd::forward_list<IDoIpSendJob> releaseJobs;
    {
        // RAII usage
        DoIpLock const lock;
        sendJobs.swap(releaseJobs);
    }
    while (!releaseJobs.empty())
    {
        IDoIpSendJob& releaseJob = releaseJobs.front();
        releaseJobs.pop_front();
        releaseJob.release(false);
    }
}

void DoIpTcpConnection::suspendSending()
{
    uint8_t prevCounter = 0U;
    {
        // RAII usage
        DoIpLock const lock;
        prevCounter         = _suspendCallCounter;
        _suspendCallCounter = prevCounter + 1U;
    }
    if (prevCounter == 0U)
    {
        // This is an optimization only, preventing spam of the `execute` method.
        (void)_sendTimeout.cancel();
    }
}

void DoIpTcpConnection::resumeSending()
{
    uint8_t prevCounter = 0U;
    {
        // RAII usage
        DoIpLock const lock;
        prevCounter = _suspendCallCounter;
        if (prevCounter == 0U)
        {
            return;
        }
        _suspendCallCounter = prevCounter - 1U;
    }
    if ((prevCounter == 1U) && (!_pendingSendJobs.empty()))
    {
        // Use `schedule` in order to make it cancelable.
        (void)::async::schedule(_context, *this, _sendTimeout, 0, ::async::TimeUnit::MILLISECONDS);
    }
}

void DoIpTcpConnection::shutdown() { (void)_sendTimeout.cancel(); }
} // namespace doip
