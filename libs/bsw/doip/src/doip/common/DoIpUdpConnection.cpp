/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpUdpConnection.h"

#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpLock.h"
#include "doip/common/IDoIpSendJob.h"

#include <async/Async.h>
#include <etl/algorithm.h>
#include <etl/intrusive_links.h>
#include <etl/intrusive_list.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/variant.h>
#include <udp/DatagramPacket.h>

namespace doip
{
using ::udp::AbstractDatagramSocket;
using ::udp::DatagramPacket;

DoIpUdpConnection::DoIpUdpConnection(
    ::async::ContextType const context,
    AbstractDatagramSocket& socket,
    etl::span<uint8_t> const writeBuffer)
: IDoIpConnection()
, IDataListener()
, _sendTimeout()
, _socket(socket)
, _currentReadBuffer()
, _writeBuffer(writeBuffer)
, _handler(nullptr)
, _localEndpoint()
, _remoteEndpoint()
, _payloadReceivedCallback()
, _pendingSendJobs()
, _availableReadDataLength(0U)
, _readPayloadLength(0U)
, _context(context)
, _headerBuffer{}
, _connectionState(ConnectionState::INIT)
, _readState(ReadState::HEADER)
, _suspendCallCounter(0U)
{}

void DoIpUdpConnection::init(IDoIpConnectionHandler& handler)
{
    if (_connectionState != ConnectionState::ACTIVE)
    {
        _handler           = &handler;
        _connectionState   = ConnectionState::ACTIVE;
        _currentReadBuffer = _headerBuffer;
        _socket.setDataListener(this);
    }
}

bool DoIpUdpConnection::receivePayload(
    etl::span<uint8_t> const payload, PayloadReceivedCallbackType const payloadReceivedCallback)
{
    if ((_readState == ReadState::PAYLOAD) && (_currentReadBuffer.size() == 0U)
        && (payload.size() <= _readPayloadLength) && payloadReceivedCallback)
    {
        _currentReadBuffer       = payload;
        _payloadReceivedCallback = payloadReceivedCallback;
        return true;
    }

    return false;
}

void DoIpUdpConnection::endReceiveMessage(
    PayloadDiscardedCallbackType const payloadDiscardedCallback)
{
    if (_readState == ReadState::PAYLOAD)
    {
        _readState         = ReadState::DISCARD;
        _currentReadBuffer = {};
    }

    // for now no special async discarding handling is implemented for UDP messages
    if (payloadDiscardedCallback.is_valid())
    {
        payloadDiscardedCallback();
    }
}

bool DoIpUdpConnection::sendMessage(IDoIpSendJob& sendJob)
{
    if ((sendJob.getDestinationEndpoint() != nullptr)
        && (_connectionState == ConnectionState::ACTIVE))
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

void DoIpUdpConnection::close() { closeConnection(ConnectionState::INACTIVE); }

void DoIpUdpConnection::dataReceived(
    ::udp::AbstractDatagramSocket& /* socket */,
    ::ip::IPAddress const sourceAddress,
    uint16_t const sourcePort,
    ::ip::IPAddress const destinationAddress,
    uint16_t const length)
{
    _readState               = ReadState::HEADER;
    _availableReadDataLength = length;
    if (length >= DoIpConstants::DOIP_HEADER_LENGTH)
    {
        _currentReadBuffer = _headerBuffer;
        _remoteEndpoint    = ::ip::IPEndpoint(sourceAddress, sourcePort);
        _localEndpoint     = ::ip::IPEndpoint(destinationAddress, _socket.getLocalPort());
        tryReceive();
        _remoteEndpoint = ::ip::IPEndpoint();
        _localEndpoint  = ::ip::IPEndpoint();
    }
    if (_availableReadDataLength > 0U)
    {
        (void)_socket.read(nullptr, _availableReadDataLength);
    }
}

void DoIpUdpConnection::execute() { trySend(); }

IDoIpSendJob* DoIpUdpConnection::popSendJob()
{
    // RAII usage
    DoIpLock const lock;
    if (_pendingSendJobs.empty())
    {
        return nullptr;
    }
    IDoIpSendJob& sendJob = _pendingSendJobs.front();
    _pendingSendJobs.pop_front();
    return &sendJob;
}

void DoIpUdpConnection::trySend()
{
    {
        // RAII usage
        DoIpLock const lock;
        if (_suspendCallCounter != 0U)
        {
            return;
        }
    }
    while (_connectionState == ConnectionState::ACTIVE)
    {
        IDoIpSendJob* const sendJob = popSendJob();
        if (sendJob == nullptr)
        {
            return;
        }
        etl::span<uint8_t const> const sendBuffer = prepareSendBuffer(*sendJob);
        if (sendBuffer.size() > 0U)
        {
            AbstractDatagramSocket::ErrorCode const result = _socket.send(DatagramPacket(
                sendBuffer.data(),
                static_cast<uint16_t>(sendBuffer.size()),
                *sendJob->getDestinationEndpoint()));
            sendJob->release(result == AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK);
        }
        else
        {
            sendJob->release(false);
        }
    }
}

void DoIpUdpConnection::tryReceive()
{
    while ((_connectionState == ConnectionState::ACTIVE) && (_currentReadBuffer.size() > 0U)
           && (_currentReadBuffer.size() <= _availableReadDataLength))
    {
        size_t const bytesRead = _socket.read(_currentReadBuffer.data(), _currentReadBuffer.size());
        if (bytesRead != _currentReadBuffer.size())
        {
            _availableReadDataLength = 0U;
            break;
        }
        etl::span<uint8_t> currentReadBuffer;
        ::std::swap(_currentReadBuffer, currentReadBuffer);
        if (_readState == ReadState::HEADER)
        {
            auto headerBuffer        = currentReadBuffer.reinterpret_as<DoIpHeader>();
            DoIpHeader const& header = headerBuffer[0];
            _readState               = ReadState::PAYLOAD;
            _readPayloadLength       = static_cast<uint16_t>(
                ::etl::min<uint32_t>(header.payloadLength, _availableReadDataLength));

            auto const headerReceivedContinuation = _handler->headerReceived(header);
            if (::etl::holds_alternative<IDoIpConnection::PayloadDiscardedCallbackType>(
                    headerReceivedContinuation))
            {
                _readState         = ReadState::DISCARD;
                _currentReadBuffer = {};

                // for now no special async discarding handling is implemented for UDP messages
                auto const payloadDiscardedCallback
                    = ::etl::get<IDoIpConnection::PayloadDiscardedCallbackType>(
                        headerReceivedContinuation);
                if (payloadDiscardedCallback)
                {
                    payloadDiscardedCallback();
                }
            }
        }
        else
        {
            _readPayloadLength -= static_cast<uint16_t>(currentReadBuffer.size());
            // No is_valid() check is done here. This will assert if the user doesn't pass a valid
            // one.
            _payloadReceivedCallback(currentReadBuffer);
        }
        // bytesRead is always less then available length
        _availableReadDataLength -= bytesRead;
    }
}

// _writeBuffer is modified
etl::span<uint8_t const> DoIpUdpConnection::fillWriteBuffer(IDoIpSendJob& job)
{
    size_t offset                 = 0U;
    uint8_t const sendBufferCount = job.getSendBufferCount();
    for (uint8_t bufferIdx = 0U; bufferIdx < sendBufferCount; ++bufferIdx)
    {
        etl::span<uint8_t> const destBuffer       = _writeBuffer.subspan(offset);
        etl::span<uint8_t const> const sendBuffer = job.getSendBuffer(destBuffer, bufferIdx);
        if (sendBuffer.size() == 0U)
        {
            // nothing
        }
        else if (sendBuffer.size() <= destBuffer.size())
        {
            if (sendBuffer.data() != destBuffer.data())
            {
                ::etl::mem_copy(sendBuffer.begin(), sendBuffer.end(), destBuffer.begin());
            }
            offset += sendBuffer.size();
        }
        else
        {
            return {};
        }
    }
    return _writeBuffer.subspan(0U, offset);
}

etl::span<uint8_t const> DoIpUdpConnection::prepareSendBuffer(IDoIpSendJob& job)
{
    uint8_t const sendBufferCount = job.getSendBufferCount();
    if (sendBufferCount == 1U)
    {
        return job.getSendBuffer(_writeBuffer, 0U);
    }

    return fillWriteBuffer(job);
}

void DoIpUdpConnection::closeConnection(ConnectionState const connectionState)
{
    if (_connectionState == ConnectionState::ACTIVE)
    {
        _connectionState                      = connectionState;
        _readState                            = ReadState::DISCARD;
        IDoIpConnectionHandler* const handler = _handler;
        _handler                              = nullptr;
        releaseSendJobs(_pendingSendJobs);
        _socket.close();
        // This is an optimization only, preventing spam of the `execute` method.
        (void)_sendTimeout.cancel();
        handler->connectionClosed(false);
    }
}

void DoIpUdpConnection::releaseSendJobs(SendJobList& sendJobs)
{
    SendJobList releaseJobs;
    {
        // RAII usage
        DoIpLock const lock;
        releaseJobs.splice(releaseJobs.end(), sendJobs);
    }
    while (!releaseJobs.empty())
    {
        IDoIpSendJob& releaseJob = releaseJobs.front();
        releaseJobs.pop_front();
        releaseJob.release(false);
    }
}

void DoIpUdpConnection::suspendSending()
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
        (void)_sendTimeout.cancel();
    }
}

void DoIpUdpConnection::resumeSending()
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
} // namespace doip
