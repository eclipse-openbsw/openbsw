// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_E2A39D8B_9F42_42D4_9681_3E007A2C9602
#define GUARD_E2A39D8B_9F42_42D4_9681_3E007A2C9602

#include "async/Async.h"

#include <gmock/gmock.h>

namespace async
{
class TimeoutMock : public TimeoutType
{
public:
    MOCK_METHOD1(cancel, void(TimeoutType&));
};
} // namespace async

#endif // GUARD_E2A39D8B_9F42_42D4_9681_3E007A2C9602
