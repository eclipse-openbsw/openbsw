// Copyright 2024 Accenture.

#pragma once

#include "can/CanPhyMock.h"

#include <bsp/can/canTransceiver/CanPhy.h>
#include <bsp/power/IEcuPowerStateController.h>
#include <can/transceiver/AbstractCANTransceiver.h>
#include <etl/delegate.h>
#include <etl/uncopyable.h>

#include <gmock/gmock.h>

namespace bios
{
class CanPhyMock;

class FlexCANDevice : public ::etl::uncopyable
{
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
        ::etl::delegate<void()> frameSentCallback,
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

    MOCK_METHOD(can::ICanTransceiver::ErrorCode, init, ());
    MOCK_METHOD(can::ICanTransceiver::ErrorCode, start, ());
    MOCK_METHOD(void, stop, ());
    MOCK_METHOD(void, mute, ());
    MOCK_METHOD(void, unmute, ());
    MOCK_METHOD(uint8_t, receiveISR, (uint8_t const* filterMap));
    MOCK_METHOD(void, transmitISR, ());
    MOCK_METHOD(void, enableTransmitInterrupt, ());
    MOCK_METHOD(void, disableTransmitInterrupt, ());
    MOCK_METHOD(uint8_t, getTransmitBuffer, (can::CANFrame const& frame, bool callbackRequested));
    MOCK_METHOD(
        can::ICanTransceiver::ErrorCode,
        transmit,
        (can::CANFrame const& frame, uint8_t bufIdx, bool txInterruptNeeded));
    MOCK_METHOD(
        can::ICanTransceiver::ErrorCode,
        transmitStream,
        (uint8_t * Txframe, bool txInterruptNeeded));
    MOCK_METHOD(uint8_t, getTxErrorCounter, ());
    MOCK_METHOD(uint8_t, getRxErrorCounter, ());
    MOCK_METHOD(BusState, getBusOffState, ());

    CanPhy& getPhy() { return fPhy; }

    // We can't easily mock this as this object is constructed before we get a
    // chance to set expectations, and getIndex() is called immediately.
    // Instead, we just provide a default implementation.
    uint8_t getIndex() { return 0; }

    MOCK_METHOD(uint32_t, getBaudrate, (), (const));
    MOCK_METHOD(can::CANFrame&, dequeueRxFrame, ());
    MOCK_METHOD(unsigned char, dequeueRxFrameStream, (unsigned char* data));
    MOCK_METHOD(can::CANFrame&, getRxFrameQueueFront, ());
    MOCK_METHOD(bool, isRxQueueEmpty, (), (const));
    MOCK_METHOD(uint32_t, getRxAlive, ());
    MOCK_METHOD(void, clearRxAlive, ());
    MOCK_METHOD(uint32_t, getFirstCanId, (), (const));
    MOCK_METHOD(void, resetFirstFrame, ());
    MOCK_METHOD(bool, wokenUp, ());

    IEcuPowerStateController& fPowerManager;
    Config const& fConfig;
    CanPhy& fPhy;
    ::etl::delegate<void()> fFrameSentCallback;
};

} // namespace bios
