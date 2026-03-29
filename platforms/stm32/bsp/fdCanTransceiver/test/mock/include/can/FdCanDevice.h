// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include <can/canframes/CANFrame.h>
#include <etl/delegate.h>
#include <stdint.h>

#include <gmock/gmock.h>

namespace bios
{

/**
 * \brief Mock FdCanDevice for unit testing FdCanTransceiver.
 *
 * Replaces the real FdCanDevice (which accesses STM32 FDCAN registers)
 * with a GMock class. Allows transceiver state-machine testing on host.
 */
class FdCanDevice
{
public:
    struct Config
    {
        uint32_t baseAddress;
        uint32_t prescaler;
        uint32_t nts1;
        uint32_t nts2;
        uint32_t nsjw;
        uint32_t rxGpioPort;
        uint8_t rxPin;
        uint8_t rxAf;
        uint32_t txGpioPort;
        uint8_t txPin;
        uint8_t txAf;
    };

    static constexpr uint32_t RX_QUEUE_SIZE = 32U;

    explicit FdCanDevice(Config const& /* config */) {}

    FdCanDevice(Config const& /* config */, ::etl::delegate<void()> callback)
    : fFrameSentCallback(callback)
    {}

    MOCK_METHOD(void, init, ());
    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());
    MOCK_METHOD(bool, transmit, (::can::CANFrame const& frame));
    MOCK_METHOD(bool, transmit, (::can::CANFrame const& frame, bool txInterruptNeeded));
    MOCK_METHOD(uint8_t, receiveISR, (uint8_t const* filterBitField));

    // Mock transmitISR — tests can set expectations on it.
    // Default action invokes the stored callback delegate (matching real FdCanDevice).
    MOCK_METHOD(void, transmitISR, ());

    void setupDefaultTransmitISR()
    {
        ON_CALL(*this, transmitISR()).WillByDefault([this]() {
            if (fFrameSentCallback.is_valid())
            {
                fFrameSentCallback();
            }
        });
    }
    MOCK_METHOD(bool, isBusOff, (), (const));
    MOCK_METHOD(uint8_t, getTxErrorCounter, (), (const));
    MOCK_METHOD(uint8_t, getRxErrorCounter, (), (const));
    MOCK_METHOD(void, configureAcceptAllFilter, ());
    MOCK_METHOD(void, configureFilterList, (uint32_t const* idList, uint8_t count));
    MOCK_METHOD(::can::CANFrame const&, getRxFrame, (uint8_t index), (const));
    MOCK_METHOD(uint8_t, getRxCount, (), (const));
    MOCK_METHOD(void, clearRxQueue, ());
    MOCK_METHOD(void, disableRxInterrupt, ());
    MOCK_METHOD(void, enableRxInterrupt, ());

    ::etl::delegate<void()> fFrameSentCallback;
};

} // namespace bios
