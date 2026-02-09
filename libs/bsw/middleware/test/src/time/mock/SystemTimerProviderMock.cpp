#include "SystemTimerProviderMock.h"

#include <cassert>

#include "middleware/time/SystemTimerProvider.h"

namespace middleware
{
namespace time
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

    std::abort();
}

uint32_t getCurrentTimeInUs()
{
    if (gSystemTimerProviderMockPtr != nullptr)
    {
        return gSystemTimerProviderMockPtr->getCurrentTimeInUs();
    }

    std::abort();
}

} // namespace time
} // namespace middleware
