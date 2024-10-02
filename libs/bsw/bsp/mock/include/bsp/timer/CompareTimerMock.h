// Copyright 2024 Accenture.

#ifndef GUARD_1B3D7F97_04DD_4FAE_9965_28D4F7B292E6
#define GUARD_1B3D7F97_04DD_4FAE_9965_28D4F7B292E6

#include "bsp/timer/ICompareTimer.h"

#include <gmock/gmock.h>

namespace bios
{
class CompareTimerMock : public ICompareTimer
{
public:
    MOCK_METHOD0(init, void());
    MOCK_METHOD2(start, uint8_t(uint32_t, bool));
    MOCK_METHOD0(stop, void());
    MOCK_METHOD0(clearIsrStatus, void());
    MOCK_METHOD0(getIsrStatus, bool());
    MOCK_METHOD1(isrEnable, void(bool));
    MOCK_METHOD0(getIsrEnableStatus, bool());
    MOCK_METHOD1(isrDisable, void(bool));
    MOCK_METHOD0(getPeriodTime, uint32_t());
    MOCK_METHOD0(getCurrentTime, uint32_t());
};

} // namespace bios

#endif /* GUARD_1B3D7F97_04DD_4FAE_9965_28D4F7B292E6 */
