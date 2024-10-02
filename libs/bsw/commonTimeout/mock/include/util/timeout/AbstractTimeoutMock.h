// Copyright 2024 Accenture.

#ifndef GUARD_551D529C_F3AE_43B3_9DE4_44C145F02257
#define GUARD_551D529C_F3AE_43B3_9DE4_44C145F02257

#include "gmock/gmock.h"
#include "util/timeout/AbstractTimeout.h"

namespace common
{
class AbstractTimeoutMock : public AbstractTimeout
{
public:
    MOCK_METHOD1(expired, void(TimeoutExpiredActions));
};

} // namespace common

#endif /* GUARD_551D529C_F3AE_43B3_9DE4_44C145F02257 */
