// Copyright 2024 Accenture.

#pragma once

#include <can/transceiver/AbstractCANTransceiver.h>
#include <io/MemoryQueue.h>

#include <atomic>

namespace can
{

/**
 * The implementation of ICanTransceiver using Linux SocketCAN stack
 *
 * The class assumes that all its methods except write(), the constructor, and the destructor are
 * run in the same task context. The deviation from this can result in unobvious UBs.
 * The transceiver state change detection is currently not implemented,
 * the corresponding callback is never called.
 */
class SocketCanTransceiver final : public AbstractCANTransceiver
{
public:
    /**
     * The transceiver device (static) configuration
     *
     * The instance of this class is provided to SocketCanTransceiver constructor
     * and is kept by reference during the lifetime of the constructed object.
     */
    struct DeviceConfig
    {
        char const* name; /// SocketCAN interface name
        uint8_t busId;    /// currently not used
    };

    explicit SocketCanTransceiver(DeviceConfig const& config);

    SocketCanTransceiver(SocketCanTransceiver const&)            = delete;
    SocketCanTransceiver& operator=(SocketCanTransceiver const&) = delete;

    ICanTransceiver::ErrorCode init() final;
    ICanTransceiver::ErrorCode open() final;
    ICanTransceiver::ErrorCode open(CANFrame const& frame) final;
    ICanTransceiver::ErrorCode close() final;
    void shutdown() final;

    ICanTransceiver::ErrorCode write(CANFrame const& frame) final;
    ICanTransceiver::ErrorCode write(CANFrame const& frame, ICANFrameSentListener& listener) final;

    ICanTransceiver::ErrorCode mute() final;
    ICanTransceiver::ErrorCode unmute() final;

    uint32_t getBaudrate() const final;
    uint16_t getHwQueueTimeout() const final;

    /**
     * This function is supposed to be run periodically.
     * The callbacks registered with the ICanTransceiver will be triggered from inside this call.
     */
    void run(int maxSentPerRun, int maxReceivedPerRun);

private:
    static size_t const TX_QUEUE_SIZE_BYTES = 512;

    struct FrameWithListener
    {
        CANFrame frame;
        ICANFrameSentListener* listener;
    };

    using TxQueue = ::io::MemoryQueue<TX_QUEUE_SIZE_BYTES, sizeof(FrameWithListener)>;

    // these functions are making system calls and shall be signal-masked
    void guardedOpen();
    void guardedClose();
    void guardedRun(int maxSentPerRun, int maxReceivedPerRun);

    TxQueue _txQueue;
    ::io::MemoryQueueReader<TxQueue> _txReader;
    ::io::MemoryQueueWriter<TxQueue> _txWriter;

    DeviceConfig const& _config;

    int _fileDescriptor;

    ::std::atomic_bool _writable;
};

} // namespace can
