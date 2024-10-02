// Copyright 2024 Accenture.

#ifndef GUARD_F9E03412_0319_483F_890D_64F56223F5C4
#define GUARD_F9E03412_0319_483F_890D_64F56223F5C4

#include "bsp/background/IBackgroundJobHandler.h"

#include <gmock/gmock.h>

namespace bsp
{
class BackgroundJobHandlerMock : public IBackgroundJobHandler
{
public:
    MOCK_METHOD1(addJob, bool(AbstractBackgroundJob&));
    MOCK_METHOD1(removeJob, bool(AbstractBackgroundJob&));
};

} // namespace bsp

#endif /* GUARD_F9E03412_0319_483F_890D_64F56223F5C4 */
