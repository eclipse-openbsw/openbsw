/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

namespace middleware::concurrency::test
{

class ConcurrencyMock
{
public:
    MOCK_METHOD(void, atomicThreadFence, ());
    MOCK_METHOD(void, spinLockOptimization, ());

    void EXPECT_FENCE_CALLS(::testing::Cardinality const& times)
    {
        EXPECT_CALL(*this, atomicThreadFence()).Times(times);
    }

    void EXPECT_SPIN_CALLS(::testing::Cardinality const& times)
    {
        EXPECT_CALL(*this, spinLockOptimization()).Times(times);
    }
};

void setConcurrencyMock(ConcurrencyMock* const ptr);
void unsetConcurrencyMock();

} // namespace middleware::concurrency::test
