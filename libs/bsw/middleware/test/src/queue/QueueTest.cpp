/********************************************************************************
 * Copyright (c) 2025, 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/queue/Queue.h"

#include <gtest/gtest.h>

namespace middleware::queue::test
{

struct FakeLock
{
    FakeLock(void volatile*) {}
};

using QUEUE_ELEMENT_TYPE     = uint32_t;
constexpr uint8_t QUEUE_SIZE = 100U;

using InternalMutexTraits
    = ::middleware::queue::QueueTraits<QUEUE_ELEMENT_TYPE, QUEUE_SIZE, test::FakeLock>;
using ExternalMutexTraits
    = ::middleware::queue::QueueTraits<QUEUE_ELEMENT_TYPE, QUEUE_SIZE, test::FakeLock, uint8_t*>;
using NoMutexTraits = ::middleware::queue::QueueTraits<QUEUE_ELEMENT_TYPE, QUEUE_SIZE>;

using TestQueue                     = ::middleware::queue::Queue<InternalMutexTraits>;
using TestQueueExternalMutex        = ::middleware::queue::Queue<ExternalMutexTraits>;
using TestQueueNoLockSpecialization = ::middleware::queue::Queue<NoMutexTraits>;

TEST(TestQueue, GetInitialSize)
{
    TestQueue t;
    EXPECT_EQ(t.size(), 0U);
}

TEST(TestQueue, IsFullTest)
{
    TestQueue t;
    EXPECT_FALSE(t.isFull());
}

TEST(TestQueue, InsertTest)
{
    TestQueue t;

    EXPECT_FALSE(t.isFull());

    TestQueue::Sender writer(t);
    writer.write(100U);
    EXPECT_EQ(t.size(), 1U);
    EXPECT_EQ(t.getStats().maxLoad, 1U);
}

TEST(TestQueue, GetInitialSizeNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;
    EXPECT_EQ(t.size(), 0U);
}

TEST(TestQueue, IsFullTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;
    EXPECT_FALSE(t.isFull());
}

TEST(TestQueue, InsertTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;

    EXPECT_FALSE(t.isFull());

    TestQueueNoLockSpecialization::Sender writer(t);
    writer.write(100U);
    EXPECT_EQ(t.size(), 1U);
    EXPECT_EQ(t.getStats().maxLoad, 1U);
}

TEST(TestQueue, MaxLoadTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;

    EXPECT_FALSE(t.isFull());

    TestQueueNoLockSpecialization::Sender writer(t);
    unsigned i = 0U;
    for (; i < 50U; ++i)
    {
        writer.write(i);
        EXPECT_EQ(t.size(), i + 1);
        EXPECT_EQ(t.getStats().maxLoad, i + 1);
    }
    TestQueueNoLockSpecialization::Receiver receiver(t);
    auto const i_tmp = i;
    while (!t.isEmpty())
    {
        auto const v = receiver.peek();
        receiver.advance();
        EXPECT_EQ(v, i_tmp - i);
        EXPECT_EQ(t.size(), --i);
        EXPECT_EQ(t.getStats().maxLoad, 50U);
    }
}

TEST(TestQueue, WritteMessagesTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;

    EXPECT_FALSE(t.isFull());

    TestQueueNoLockSpecialization::Sender writer(t);
    TestQueueNoLockSpecialization::Receiver receiver(t);
    unsigned i = 0U;
    for (; i < 50U; ++i)
    {
        writer.write(i);
        EXPECT_EQ(t.size(), i + 1);
        EXPECT_EQ(t.getStats().maxLoad, i + 1);
    }
    EXPECT_EQ(t.getStats().processedMessages, 0U);
    while (i-- > 0U)
    {
        auto m = receiver.peek();
        receiver.advance();
        (void)m;
    }
    EXPECT_EQ(t.getStats().processedMessages, 50U);
}

TEST(TestQueue, LostMessagesTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;

    EXPECT_FALSE(t.isFull());

    TestQueueNoLockSpecialization::Sender writer(t);
    unsigned i = 0U;
    for (; i < 150U; ++i)
    {
        if (i < 100)
        {
            EXPECT_TRUE(writer.write(i));
            EXPECT_EQ(t.size(), i + 1);
            EXPECT_EQ(t.getStats().maxLoad, i + 1);
        }
        else
        {
            EXPECT_FALSE(writer.write(i));
            EXPECT_EQ(t.size(), 100U);
            EXPECT_EQ(t.getStats().maxLoad, 100U);
            EXPECT_EQ(t.getStats().lostMessages, 1U + (i - 100U));
        }
    }
    TestQueueNoLockSpecialization::Receiver receiver(t);
    auto const i_tmp = i;
    while (!t.isEmpty())
    {
        auto const v = receiver.peek();
        receiver.advance();
        EXPECT_EQ(v, i_tmp - i);
        EXPECT_EQ(t.size(), --i - 50U);
        EXPECT_EQ(t.getStats().maxLoad, 100U);
        EXPECT_EQ(t.getStats().lostMessages, 50U);
    }
}

TEST(TestExternalMutexTraits, ExternalMutexTest)
{
    uint8_t volatile mutex{0xFFU};
    QueueMutex<ExternalMutexTraits::MutexType> testingMutex{&mutex};
    EXPECT_EQ(*testingMutex.get(), 0xFFU);
}

} // namespace middleware::queue::test
