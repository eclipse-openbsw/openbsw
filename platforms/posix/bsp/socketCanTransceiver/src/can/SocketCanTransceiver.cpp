// Copyright 2024 Accenture.

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
    ::std::strcpy(ifr.ifr_name, name);
    error = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (error < 0)
    {
        Logger::error(
            CAN, "[SocketCanTransceiver] Failed to ioctl socket (node=%s, error=%d)", name, error);
        return;
    }

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
    error            = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (error < 0)
    {
        Logger::error(
            CAN, "[SocketCanTransceiver] Failed to bind socket (node=%s, error=%d)", name, error);
        return;
    }

    _fileDescriptor = fd;
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
        can_frame socketCanFrame;
        ::std::memset(&socketCanFrame, 0, sizeof(socketCanFrame));
        socketCanFrame.can_id  = canFrame.getId();
        int length             = canFrame.getPayloadLength();
        socketCanFrame.can_dlc = length;
        ::std::memcpy(socketCanFrame.data, canFrame.getPayload(), length);
        ::std::memset(socketCanFrame.data + length, 0, sizeof(socketCanFrame.data) - length);
        ssize_t const bytesWritten
            = ::write(_fileDescriptor, reinterpret_cast<char*>(&socketCanFrame), CAN_MTU);
        if (bytesWritten != CAN_MTU)
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
        alignas(can_frame) uint8_t buffer[CANFD_MTU];
        ssize_t const length = read(_fileDescriptor, reinterpret_cast<char*>(buffer), CANFD_MTU);
        if (length < 0)
        {
            break;
        }
        if (length == CAN_MTU)
        {
            can_frame const& socketCanFrame = *reinterpret_cast<can_frame const*>(buffer);
            Logger::debug(
                CAN,
                "[SocketCanTransceiver] received CAN frame, id=0x%X, length=%d",
                (int)socketCanFrame.can_id,
                (int)socketCanFrame.can_dlc);
            CANFrame canFrame;
            canFrame.setId(socketCanFrame.can_id);
            canFrame.setPayload(socketCanFrame.data, socketCanFrame.can_dlc);
            canFrame.setPayloadLength(socketCanFrame.can_dlc);
            canFrame.setTimestamp(0);

            notifyListeners(canFrame);
        }
    }
}

} // namespace can
