#pragma once

#include <cstdint>

#include <gmock/gmock.h>

namespace middleware
{
namespace time
{
namespace test
{

class SystemTimerProviderMock
{
public:
    MOCK_METHOD(uint32_t, getCurrentTimeInMs, ());
    MOCK_METHOD(uint32_t, getCurrentTimeInUs, ());
};

void setSystemTimerProviderMock(SystemTimerProviderMock* const ptr);
void unsetSystemTimerProviderMock();

} // namespace test
} // namespace time
} // namespace middleware
