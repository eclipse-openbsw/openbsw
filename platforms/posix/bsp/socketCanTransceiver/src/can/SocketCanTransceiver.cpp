/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/SocketCanTransceiver.h"

#include <can/CanLogger.h>
#include <can/canframes/ICANFrameSentListener.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <signal.h>
#include <type_traits>
#include <unistd.h>

#include <etl/char_traits.h>
#include <etl/error_handler.h>
#include <etl/span.h>
#include <sys/types.h>

static_assert(
    std::is_standard_layout<::can::CANFrame>::value
        && std::is_trivially_destructible<::can::CANFrame>::value,
    "check for UB while passing through TxQueue");

namespace can
{

using ::util::logger::CAN;
using ::util::logger::Logger;

namespace
{

template<typename F>
void signalGuarded(F&& function)
{
    sigset_t set, oldSet;
    sigfillset(&set);
    pthread_sigmask(SIG_SETMASK, &set, &oldSet);
    ::std::forward<F>(function)();
    pthread_sigmask(SIG_SETMASK, &oldSet, nullptr);
}

} // namespace

// needed if ODR-used
size_t const SocketCanTransceiver::TX_QUEUE_SIZE_BYTES;

SocketCanTransceiver::SocketCanTransceiver(DeviceConfig const& config)
: AbstractCANTransceiver(config.busId)
, _txQueue()
, _txReader(_txQueue)
, _txWriter(_txQueue)
, _config(config)
, _fileDescriptor(-1)
, _writable(false)
{}

ICanTransceiver::ErrorCode SocketCanTransceiver::init()
{
    if (!isInState(State::CLOSED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::INITIALIZED);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode SocketCanTransceiver::open()
{
    if (!isInState(State::INITIALIZED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    signalGuarded([this] { guardedOpen(); });
    setState(State::OPEN);
    _writable.store(true);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode SocketCanTransceiver::open(CANFrame const& /* frame */)
{
    ETL_ASSERT_FAIL(ETL_ERROR_GENERIC("not implemented"));
    return ErrorCode::CAN_ERR_ILLEGAL_STATE;
}

ICanTransceiver::ErrorCode SocketCanTransceiver::close()
{
    if (!isInState(State::OPEN) && !isInState(State::MUTED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    signalGuarded([this] { guardedClose(); });
    _writable.store(false);
    setState(State::CLOSED);
    return ErrorCode::CAN_ERR_OK;
}

void SocketCanTransceiver::shutdown() {}

ICanTransceiver::ErrorCode SocketCanTransceiver::write(CANFrame const& frame)
{
    return writeImpl(frame, nullptr);
}

ICanTransceiver::ErrorCode
SocketCanTransceiver::write(CANFrame const& frame, ICANFrameSentListener& listener)
{
    return writeImpl(frame, &listener);
}

ICanTransceiver::ErrorCode
SocketCanTransceiver::writeImpl(CANFrame const& frame, ICANFrameSentListener* listener)
{
    if (!_writable.load(std::memory_order_relaxed))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    auto memory = _txWriter.allocate(TX_ELEMENT_SIZE_BYTES);
    if (memory.size() < TX_ELEMENT_SIZE_BYTES)
    {
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }
    ::std::memcpy(memory.data(), &frame, sizeof(frame));
    ::std::memcpy(memory.data() + sizeof(frame), static_cast<void*>(&listener), sizeof(void*));
    _txWriter.commit();
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode SocketCanTransceiver::mute()
{
    if (!isInState(State::OPEN))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    _writable.store(false);
    setState(State::MUTED);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode SocketCanTransceiver::unmute()
{
    if (!isInState(State::MUTED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    _writable.store(true);
    return ErrorCode::CAN_ERR_OK;
}

uint32_t SocketCanTransceiver::getBaudrate() const { return 500000U; }

uint16_t SocketCanTransceiver::getHwQueueTimeout() const { return 1U; }

void SocketCanTransceiver::run(int maxSentPerRun, int maxReceivedPerRun)
{
    signalGuarded([this, maxSentPerRun, maxReceivedPerRun]
                  { guardedRun(maxSentPerRun, maxReceivedPerRun); });
}

void SocketCanTransceiver::guardedOpen()
{
    // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API is variadic by design.
    char const* const name = _config.name;
    int error              = 0;
    int const fd           = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd < 0)
    {
        Logger::error(
            CAN, "[SocketCanTransceiver] Failed to create socket (node=%s, error=%d)", name, fd);
        return;
    }

    struct ifreq ifr;
    etl::strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name) / sizeof(ifr.ifr_name[0]));
    error = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (error < 0)
    {
        Logger::error(
            CAN, "[SocketCanTransceiver] Failed to ioctl socket (node=%s, error=%d)", name, error);
        return;
    }

    if (_config.enableCanFd)
    {
        int const enable_canfd = 1;
        error = setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd));
        if (error < 0)
        {
            Logger::error(
                CAN,
                "[SocketCanTransceiver] Failed to setsockopt socket (node=%s, error=%d)",
                name,
                error);
            return;
        }
    }

    error = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (error < 0)
    {
        Logger::error(
            CAN,
            "[SocketCanTransceiver] Failed to switch to non-blocking mode (node=%s, error=%d)",
            name,
            error);
        return;
    }

    struct sockaddr_can addr;
    ::std::memset(&addr, 0, sizeof(addr));
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): POSIX bind() requires sockaddr*
    error            = bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (error < 0)
    {
        Logger::error(
            CAN, "[SocketCanTransceiver] Failed to bind socket (node=%s, error=%d)", name, error);
        return;
    }

    _fileDescriptor = fd;
    // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

void SocketCanTransceiver::guardedClose()
{
    ::close(_fileDescriptor);
    _fileDescriptor = -1;
}

void SocketCanTransceiver::guardedRun(int maxSentPerRun, int maxReceivedPerRun)
{
    // MUTED condition does not affect the messages already in the write queue;
    // the idea is that once we confirmed that we had accepted the message for delivery,
    // we shall try to deliver it.
    for (int count = 0; count < maxSentPerRun; ++count)
    {
        auto memory = _txReader.peek();
        if (memory.size() < TX_ELEMENT_SIZE_BYTES)
        {
            break;
        }
        CANFrame canFrame;
        ::std::memcpy(static_cast<void*>(&canFrame), memory.data(), sizeof(canFrame));
        ICANFrameSentListener* listener = nullptr;
        ::std::memcpy(
            static_cast<void*>(&listener), memory.data() + sizeof(canFrame), sizeof(void*));
        _txReader.release();

        uint8_t const length = static_cast<uint8_t>(canFrame.getPayloadLength());
        bool const sendAsFd  = _config.enableCanFd;

        // canfd_frame is layout-compatible with can_frame for the fields we set
        // (can_id, len, data); byte 5 (flags) maps to can_frame::__pad for
        // classical frames and must remain 0, which memset guarantees.
        canfd_frame outFrame;
        ::std::memset(&outFrame, 0, sizeof(outFrame));
        outFrame.can_id = canFrame.getId();
        outFrame.len    = length;
        if (sendAsFd && _config.enableBitRateSwitch)
        {
            outFrame.flags |= CANFD_BRS;
        }
        ::std::memcpy(outFrame.data, canFrame.getPayload(), length);

        // MTU selects the on-wire frame type: CAN_MTU for classical, CANFD_MTU for FD.
        size_t const mtu           = sendAsFd ? CANFD_MTU : CAN_MTU;
        ssize_t const bytesWritten = ::write(_fileDescriptor, &outFrame, mtu);
        if (bytesWritten != static_cast<ssize_t>(mtu))
        {
            break;
        }
        if (listener != nullptr)
        {
            listener->canFrameSent(canFrame);
        }
        notifySentListeners(canFrame);
    }

    for (int count = 0; count < maxReceivedPerRun; ++count)
    {
        canfd_frame inFrame;
        ::std::memset(&inFrame, 0, sizeof(inFrame));
        ssize_t const bytesRead = ::read(_fileDescriptor, &inFrame, CANFD_MTU);
        if (bytesRead < 0)
        {
            break;
        }
        if (bytesRead != CAN_MTU && bytesRead != CANFD_MTU)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): Logger API is variadic by design.
            Logger::warn(
                CAN,
                "[SocketCanTransceiver] discarded frame with unexpected size=%d",
                static_cast<int>(bytesRead));
            continue;
        }

        CANFrame canFrame;
        canFrame.setId(inFrame.can_id);
        canFrame.setPayload(inFrame.data, inFrame.len);
        canFrame.setTimestamp(0);

        notifyListeners(canFrame);
    }
}

} // namespace can
