// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_97468D82_9418_4F5C_B77D_6CB64B53D336
#define GUARD_97468D82_9418_4F5C_B77D_6CB64B53D336

#include "async/Async.h"
#include "estd/functional.h"
#include "estd/singleton.h"

#include <gmock/gmock.h>

namespace async
{
class TimeoutMock : public ::estd::singleton<TimeoutMock>
{
public:
    TimeoutMock() : ::estd::singleton<TimeoutMock>(*this) {}

    MOCK_METHOD1(cancel, void(TimeoutType& timeout));
};

} // namespace async

#endif // GUARD_97468D82_9418_4F5C_B77D_6CB64B53D336
