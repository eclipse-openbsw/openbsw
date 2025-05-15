// Copyright 2024 Accenture.

#pragma once

#include "io/Io.h"

#include <bsp/Bsp.h>
#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace bios
{
struct IoMock : public ::etl::singleton_base<IoMock>
{
    IoMock() : ::etl::singleton_base<IoMock>(*this) {}

    MOCK_METHOD(bsp::BspReturnCode, setDefaultConfiguration, (uint16_t));
    MOCK_METHOD(bool, getPin, (uint16_t));
    MOCK_METHOD(bsp::BspReturnCode, setPin, (uint16_t, bool));

    MOCK_METHOD(uint32_t, getPinNumber, (Io::PinId));
    MOCK_METHOD(bsp::BspReturnCode, resetConfig, (uint16_t));

    MOCK_METHOD(bsp::BspReturnCode, safe, (uint16_t, bool));
    MOCK_METHOD(bool, getPinIrq, (uint16_t));
};

} // namespace bios
