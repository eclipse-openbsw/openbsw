#pragma once

#include <cstdint>

#include <gmock/gmock.h>

namespace middleware::time::test
{

class SystemTimerProviderMock
{
public:
    MOCK_METHOD(uint32_t, getCurrentTimeInMs, ());
    MOCK_METHOD(uint32_t, getCurrentTimeInUs, ());
};

void setSystemTimerProviderMock(SystemTimerProviderMock* const ptr);
void unsetSystemTimerProviderMock();

} // namespace middleware::time::test
