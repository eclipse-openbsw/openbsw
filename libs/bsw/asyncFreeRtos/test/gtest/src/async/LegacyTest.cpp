// Copyright 2024 Accenture.

#include "async/Legacy.h"

#include "async/Async.h"
#include "async/AsyncBinding.h"

namespace
{
using namespace ::async;
using namespace ::testing;

using AdapterType = AsyncBindingType::AdapterType;

/**
 * \refs: SMD_asyncFreeRtos_FreeRtosAdapterLegacy
 * \desc: To test Timeout Manager and Looper
 */
TEST(LegacyTest, testGetTimeoutManagerAndLooper)
{
#ifndef ASYNC_TIMEOUTMANAGER2_DISABLE
    EXPECT_EQ(&AdapterType::getTimeoutManager(1U), &getTimeoutManager(1U));
#endif // ASYNC_TIMEOUTMANAGER2_DISABLE

#ifndef ASYNC_LOOPER_DISABLE
    EXPECT_EQ(&AdapterType::getLooper(1U), &getLooper(1U));
#endif // ASYNC_LOOPER_DISABLE
}

} // namespace
