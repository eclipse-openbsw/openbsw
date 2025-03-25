// Copyright 2024 Accenture.

#pragma once

#include "can/CanPhyMock.h"

#include <bsp/can/canTransceiver/CanPhy.h>
#include <bsp/power/IEcuPowerStateController.h>
#include <can/transceiver/AbstractCANTransceiver.h>

#include <estd/uncopyable.h>

#include <gmock/gmock.h>

namespace bios
{
class CanPhyMock;

class FlexCANDevice
{
    UNCOPYABLE(FlexCANDevice);

public:
    struct Config
    {
        uint32_t baseAddress;
        uint32_t baudrate;
        uint16_t txPort;
        uint16_t rxPort;
        uint8_t numRxBufsStd;
        uint8_t numRxBufsExt;
        uint8_t numTxBufsApp;
        uint32_t BusId;
        uint32_t wakeUp;
        uint8_t disableAutomaticBusOffRecovery;
    };

    enum BusState
    {
        BUS_OFF = 0,
        BUS_ON
    };

    static uint8_t const TRANSMIT_BUFFER_UNAVAILABLE = 255;

    FlexCANDevice(
        Config const& config,
        CanPhy& canPhy,
        ::estd::function<void()> frameSentCallback,
        IEcuPowerStateController& powerManager)
    :

        fPowerManager(powerManager)
    , fConfig(config)
    , fPhy(canPhy)
    , fFrameSentCallback(frameSentCallback)
    {
        reinterpret_cast<bios::CanPhyMock*>(&canPhy)->setFlexCANDevice(this);
    }

    FlexCANDevice(Config const& config, CanPhy& canPhy, IEcuPowerStateController& powerManager)
    :

        fPowerManager(powerManager)
    , fConfig(config)
    , fPhy(canPhy)
    {
        reinterpret_cast<bios::CanPhyMock*>(&canPhy)->setFlexCANDevice(this);
    }

    MOCK_METHOD0(init, can::ICanTransceiver::ErrorCode());
    MOCK_METHOD0(start, can::ICanTransceiver::ErrorCode());
    MOCK_METHOD0(stop, void());
    MOCK_METHOD0(mute, void());
    MOCK_METHOD0(unmute, void());
    MOCK_METHOD1(receiveISR, uint8_t(uint8_t const* filterMap));
    MOCK_METHOD0(transmitISR, void());
    MOCK_METHOD0(enableTransmitInterrupt, void());
    MOCK_METHOD0(disableTransmitInterrupt, void());
    MOCK_METHOD2(getTransmitBuffer, uint8_t(can::CANFrame const& frame, bool callbackRequested));
    MOCK_METHOD3(
        transmit,
        can::ICanTransceiver::ErrorCode(
            can::CANFrame const& frame, uint8_t bufIdx, bool txInterruptNeeded));
    MOCK_METHOD2(
        transmitStream, can::ICanTransceiver::ErrorCode(uint8_t* Txframe, bool txInterruptNeeded));
    MOCK_METHOD0(getTxErrorCounter, uint8_t());
    MOCK_METHOD0(getRxErrorCounter, uint8_t());
    MOCK_METHOD0(getBusOffState, BusState());

    CanPhy& getPhy() { return fPhy; }

    // We can't easily mock this as this object is constructed before we get a
    // chance to set expectations, and getIndex() is called immediately.
    // Instead, we just provide a default implementation.
    uint8_t getIndex() { return 0; }

    MOCK_CONST_METHOD0(getBaudrate, uint32_t());
    MOCK_METHOD0(dequeueRxFrame, can::CANFrame&());
    MOCK_METHOD1(dequeueRxFrameStream, unsigned char(unsigned char* data));
    MOCK_METHOD0(getRxFrameQueueFront, can::CANFrame&());
    MOCK_CONST_METHOD0(isRxQueueEmpty, bool());
    MOCK_METHOD0(getRxAlive, uint32_t());
    MOCK_METHOD0(clearRxAlive, void());
    MOCK_CONST_METHOD0(getFirstCanId, uint32_t());
    MOCK_METHOD0(resetFirstFrame, void());
    MOCK_METHOD0(wokenUp, bool());

    IEcuPowerStateController& fPowerManager;
    Config const& fConfig;
    CanPhy& fPhy;
    ::estd::function<void()> fFrameSentCallback;
};

} // namespace bios
