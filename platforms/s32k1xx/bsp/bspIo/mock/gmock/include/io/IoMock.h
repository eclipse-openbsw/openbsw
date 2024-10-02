// Copyright 2024 Accenture.

#ifndef GUARD_2197BDBF_677B_47C0_B8B7_43B69E5E50FB
#define GUARD_2197BDBF_677B_47C0_B8B7_43B69E5E50FB

#include "io/Io.h"

#include <bsp/Bsp.h>

#include <estd/singleton.h>

#include <gmock/gmock.h>

namespace bios
{
struct IoMock : public ::estd::singleton<IoMock>
{
    IoMock() : ::estd::singleton<IoMock>(*this){};

    MOCK_METHOD1(setDefaultConfiguration, bsp::BspReturnCode(uint16_t));
    MOCK_METHOD1(getPin, bool(uint16_t));
    MOCK_METHOD2(setPin, bsp::BspReturnCode(uint16_t, bool));

    MOCK_METHOD1(getPinNumber, uint32_t(Io::PinId));
    MOCK_METHOD1(resetConfig, bsp::BspReturnCode(uint16_t));

    MOCK_METHOD2(safe, bsp::BspReturnCode(uint16_t, bool));
    MOCK_METHOD1(getPinIrq, bool(uint16_t));
};

} // namespace bios
#endif // GUARD_2197BDBF_677B_47C0_B8B7_43B69E5E50FB
