// Copyright 2024 Accenture.

#ifndef GUARD_EBBC9665_9D66_4BF4_948F_F97F4F271A38
#define GUARD_EBBC9665_9D66_4BF4_948F_F97F4F271A38

#include "bsp/background/AbstractBackgroundJob.h"

#include <gmock/gmock.h>

namespace bsp
{
class AbstractBackgroundJobMock : public AbstractBackgroundJob
{
public:
    MOCK_METHOD0(execute, bool());
    MOCK_METHOD0(removed, void());
};

} // namespace bsp

#endif /* GUARD_EBBC9665_9D66_4BF4_948F_F97F4F271A38 */
