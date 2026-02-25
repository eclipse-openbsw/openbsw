#include "SystemTimerProviderMock.h"

#include "etl/error_handler.h"

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

    ETL_ASSERT_FAIL("SystemTimerProviderMock is not set.");
}

uint32_t getCurrentTimeInUs()
{
    if (gSystemTimerProviderMockPtr != nullptr)
    {
        return gSystemTimerProviderMockPtr->getCurrentTimeInUs();
    }

    ETL_ASSERT_FAIL("SystemTimerProviderMock is not set.");
}

} // namespace time
} // namespace middleware
