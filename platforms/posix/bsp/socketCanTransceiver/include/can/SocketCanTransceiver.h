/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <can/transceiver/AbstractCANTransceiver.h>
#include <io/MemoryQueue.h>

#include <etl/atomic.h>
#include <can/transceiver/ICANTransceiverStateListener.h>

namespace can
{

/**
 * The implementation of ICanTransceiver using Linux SocketCAN stack
 *
 * The class assumes that all its methods except write(), the constructor, and the destructor are
 * run in the same task context. The deviation from this can result in unobvious UBs.
 * Transceiver state change detection is implemented via CAN error frames: the socket is opened with CAN_RAW_ERR_FILTER for CAN_ERR_BUSOFF | CAN_ERR_CRTL so bus-off and recovery events are delivered as SocketCAN error frames and translated into ICANTransceiverStateListener callbacks.
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
        char const* name;         /// SocketCAN interface name
        uint8_t busId;            /// currently not used
        bool enableCanFd;         /// open the socket in CAN FD mode and send frames as FD
        bool enableBitRateSwitch; /// use Bit Rate Switch (data-phase bitrate) on TX FD frames
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

    /**
     * Models bus-off recovery by closing and reopening the SocketCAN socket,
     * draining any queued error frames from the kernel RX buffer.
     * Call this once CanBusOffPolicy signals a recovery edge.
     */
    void manualBusOffRecovery();

    /**
     * Register (or unregister with nullptr) a state-change listener.
     * The listener receives BUS_OFF and ACTIVE notifications translated
     * from SocketCAN error frames during run().
     */
    void setStateListener(ICANTransceiverStateListener& listener) { _stateListener = &listener; }
    void removeStateListener() { _stateListener = nullptr; }

private:
    static constexpr size_t TX_NUM_ELEMENTS       = 16U;
    static constexpr size_t TX_ELEMENT_SIZE_BYTES = sizeof(CANFrame) + sizeof(void*);

    using ElementSizeType = uint16_t;
    static constexpr size_t TX_QUEUE_SIZE_BYTES
        = TX_NUM_ELEMENTS * (TX_ELEMENT_SIZE_BYTES + sizeof(ElementSizeType));

    // NOTE: internally also the size of each element is stored, which is reflected in the total
    // queue size but not in the configured element size
    using TxQueue = ::io::MemoryQueue<TX_QUEUE_SIZE_BYTES, TX_ELEMENT_SIZE_BYTES, ElementSizeType>;

    ICanTransceiver::ErrorCode writeImpl(CANFrame const& frame, ICANFrameSentListener* listener);

    // these functions are making system calls and shall be signal-masked
    void guardedOpen();
    void guardedClose();
    void guardedRun(int maxSentPerRun, int maxReceivedPerRun);
    void handleErrorFrame(uint32_t canId, uint8_t const* data, uint8_t length);

    TxQueue _txQueue;
    ::io::MemoryQueueReader<TxQueue> _txReader;
    ::io::MemoryQueueWriter<TxQueue> _txWriter;

    DeviceConfig const& _config;

    int _fileDescriptor;

    ::etl::atomic_bool _writable;

    ICANTransceiverStateListener* _stateListener{nullptr};
    ICANTransceiverStateListener::CANTransceiverState _transceiverState{
        ICANTransceiverStateListener::CANTransceiverState::ACTIVE};
};

} // namespace can

