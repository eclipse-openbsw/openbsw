// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */
#ifndef GUARD_E31C2E39_3CB8_4106_8BB7_482D7D55B2AE
#define GUARD_E31C2E39_3CB8_4106_8BB7_482D7D55B2AE
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
#endif /* GUARD_E31C2E39_3CB8_4106_8BB7_482D7D55B2AE */
