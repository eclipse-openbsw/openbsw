// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include <can/canframes/CANFrame.h>
#include <stdint.h>

#include <gmock/gmock.h>

namespace bios
{

/**
 * \brief Mock BxCanDevice for unit testing BxCanTransceiver.
 *
 * Replaces the real BxCanDevice (which accesses STM32 bxCAN registers)
 * with a GMock class. Allows transceiver state-machine testing on host.
 */
class BxCanDevice
{
public:
    struct Config
    {
        uint32_t baseAddress;
        uint32_t prescaler;
        uint32_t bs1;
        uint32_t bs2;
        uint32_t sjw;
        uint32_t rxGpioPort;
        uint8_t rxPin;
        uint8_t rxAf;
        uint32_t txGpioPort;
        uint8_t txPin;
        uint8_t txAf;
    };

    static constexpr uint32_t RX_QUEUE_SIZE = 32U;

    explicit BxCanDevice(Config const& /* config */) {}

    MOCK_METHOD(void, init, ());
    MOCK_METHOD(void, start, ());
    MOCK_METHOD(void, stop, ());
    MOCK_METHOD(bool, transmit, (::can::CANFrame const& frame));
    MOCK_METHOD(uint8_t, receiveISR, (uint8_t const* filterBitField));
    MOCK_METHOD(void, transmitISR, ());
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
};

} // namespace bios
