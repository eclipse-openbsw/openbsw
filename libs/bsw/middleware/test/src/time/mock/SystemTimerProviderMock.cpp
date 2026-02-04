#include "SystemTimerProviderMock.h"

#include <cassert>

#include "middleware/time/SystemTimerProvider.h"

namespace middleware::time
{

namespace
{
test::SystemTimerProviderMock* gSystemTimerProviderMockPtr = nullptr;
} // namespace

namespace test
{

void setSystemTimerProviderMock(SystemTimerProviderMock* const ptr)
{
    gSystemTimerProviderMockPtr = ptr;
}

void unsetSystemTimerProviderMock() { gSystemTimerProviderMockPtr = nullptr; }

} // namespace test

uint32_t getCurrentTimeInMs()
{
    if (gSystemTimerProviderMockPtr != nullptr)
    {
        return gSystemTimerProviderMockPtr->getCurrentTimeInMs();
    }

    assert(false);
}

uint32_t getCurrentTimeInUs()
{
    if (gSystemTimerProviderMockPtr != nullptr)
    {
        return gSystemTimerProviderMockPtr->getCurrentTimeInUs();
    }

    assert(false);
}

} // namespace middleware::time
