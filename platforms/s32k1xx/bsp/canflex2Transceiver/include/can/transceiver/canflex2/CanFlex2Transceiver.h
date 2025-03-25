// Copyright 2024 Accenture.

/**
 * Contains Cpp2CAN CanFlex2Transceiver
 * \file CanFlex2Transceiver.h
 * \ingroup transceiver
 */
#pragma once

#include <async/Async.h>
#include <async/util/Call.h>
#include <bsp/can/canTransceiver/CanPhy.h>
#include <bsp/power/IEcuPowerStateController.h>
#include <bsp/timer/SystemTimer.h>
#include <can/FlexCANDevice.h>
#include <can/canframes/BufferedCANFrame.h>
#include <can/canframes/ICANFrameSentListener.h>
#include <can/framemgmt/IFilteredCANFrameSentListener.h>
#include <can/transceiver/AbstractCANTransceiver.h>
#include <io/Io.h>
#include <lifecycle/AsyncLifecycleComponent.h>

#include <estd/deque.h>
#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace bios
{
/**
 * CAN transceiver.
 * \see AbstractCANTransceiver
 * \see FlexCANDevice
 * \see common::AbstractTimeout
 */
class CanFlex2Transceiver : public ::can::AbstractCANTransceiver
{
    UNCOPYABLE(CanFlex2Transceiver);

public:
    CanFlex2Transceiver(
        ::async::ContextType context,
        uint8_t busId,
        FlexCANDevice::Config const& devConfig,
        CanPhy& Phy,
        IEcuPowerStateController& powerStateController);

    ::can::ICanTransceiver::ErrorCode init() override;
    ::can::ICanTransceiver::ErrorCode open(::can::CANFrame const& frame) override;
    ::can::ICanTransceiver::ErrorCode open() override;

    ::can::ICanTransceiver::ErrorCode close() override;

    void shutdown() override;

    ::can::ICanTransceiver::ErrorCode write(can::CANFrame const& frame) override;

    ::can::ICanTransceiver::ErrorCode
    write(can::CANFrame const& frame, can::ICANFrameSentListener& listener) override;

    ErrorCode mute() override;

    ErrorCode unmute() override;

    /**
     * Checks if the CAN device associated with the transceiver has woken up.
     */
    bool wokenUp() { return fFlexCANDevice.wokenUp(); }

    uint32_t getBaudrate() const override { return fFlexCANDevice.getBaudrate(); }

    /**
     * \return the Hardware Queue Timeout.
     */
    uint16_t getHwQueueTimeout() const override;

    /**
     * \return ID of first frame seen after open() or resetFirstFrame() or INVALID_FRAME_ID.
     */
    virtual uint16_t getFirstFrameId() const;

    /**
     * Resets information about the first frame, the next frame received will be the first frame
     */
    virtual void resetFirstFrame();

    /**
     * Checks if the CAN transceiver is receiving data.
     */
    virtual bool getRxAlive() const
    {
        if (_state == State::MUTED)
        {
            return ((fRxAlive) || (fFlexCANDevice.getFirstCanId() != 0));
        }
        else
        {
            return fRxAlive;
        }
    }

    /**
     * \return the count of overrun events that have occurred in the CAN transceiver.
     */
    uint32_t getOverrunCount() const { return fOverrunCount; }

    /**
     * cyclicTask()
     *
     * Called periodically.
     * Checks transceiver error pins and bus state.
     * Notifies state change and phy error listeners.
     */
    void cyclicTask();

    /**
     * Processes the receive queue of the CAN device.
     */
    void receiveTask();

    /**
     * Handles sending of further frames if the transmit queue is not yet empty.
     */
    void canFrameSentCallback();
    void canFrameSentAsyncCallback();

    /**
     * Calls the Receive ISR
     * \return the number of frames received
     */
    static uint8_t receiveInterrupt(uint8_t const transceiverIndex)
    {
        return fpCanTransceivers[transceiverIndex]->fFlexCANDevice.receiveISR(
            fpCanTransceivers[transceiverIndex]->_filter.getRawBitField());
    }

    /**
     * Calls the Transmit ISR
     */
    static void transmitInterrupt(uint8_t const transceiverIndex)
    {
        fpCanTransceivers[transceiverIndex]->fFlexCANDevice.transmitISR();
    }

    /**
     * \return a list of all CanFlex2Transceiver
     */
    static CanFlex2Transceiver** getAllCanFlex2Transceivers() { return fpCanTransceivers; }

private:
    /** polling time */
    static uint32_t const ERROR_POLLING_TIMEOUT = 10;

    struct TxJobWithCallback
    {
        TxJobWithCallback(::can::ICANFrameSentListener& listener, ::can::CANFrame const& frame)
        : _listener(listener), _frame(frame)
        {}

        ::can::ICANFrameSentListener& _listener;
        ::can::CANFrame const& _frame;
    };

    using TxQueue = ::estd::declare::deque<TxJobWithCallback, 3>;

    static CanFlex2Transceiver* fpCanTransceivers[8];
    FlexCANDevice::Config const& fdevConfig;
    FlexCANDevice fFlexCANDevice;
    bool fIsPhyErrorPresent;

    uint16_t fTxOfflineErrors;
    uint32_t fOverrunCount;
    uint32_t fFramesSentCount;

    TxQueue fTxQueue;

    bool fFirstFrameNotified;
    bool fRxAlive;

    ::async::ContextType const _context;
    ::async::Function _cyclicTask;
    ::async::TimeoutType _cyclicTaskTimeout;
    ::async::Function _canFrameSent;

    ::can::ICanTransceiver::ErrorCode
    write(::can::CANFrame const& frame, ::can::ICANFrameSentListener* pListener);

    void notifyRegisteredSentListener(::can::CANFrame const& frame) { notifySentListeners(frame); }
};

} // namespace bios

