// Copyright 2025 BMW AG

#include "middleware/queue/Queue.h"

#include <gtest/gtest.h>

namespace middleware
{
namespace queue
{
namespace test
{

struct FakeLock
{
    FakeLock(void volatile*) {}
};

using TestQueue
    = ::middleware::queue::Queue<::middleware::queue::QueueTraits<uint32_t, 100U, test::FakeLock>>;
using TestQueueExternalMutex = ::middleware::queue::Queue<
    ::middleware::queue::QueueTraits<uint32_t, 100U, test::FakeLock, uint8_t*>>;
using TestQueueNoLockSpecialization
    = ::middleware::queue::Queue<::middleware::queue::QueueTraits<uint32_t, 100U>>;

TEST(TestQueue, EmptyConstructor)
{
    TestQueue queue{};
    queue.init();
    TestQueue::Sender(queue).write(0U);
    EXPECT_EQ(queue.size(), 1U);
    new (&queue) TestQueue();
    EXPECT_EQ(queue.size(), 1U);

    static_cast<void>(TestQueue::Receiver(queue).peek());
    TestQueue::Receiver(queue).advance();
    EXPECT_EQ(queue.getStats().processedMessages, 1U);
    new (&queue) TestQueue();
    EXPECT_EQ(queue.getStats().processedMessages, 1U);

    TestQueue queue2 = queue;
    EXPECT_EQ(queue2.getStats().processedMessages, 1U);
}

TEST(TestQueue, GetInitialSize)
{
    TestQueue t;
    t.init();
    EXPECT_EQ(t.size(), 0U);
}

TEST(TestQueue, IsFullTest)
{
    TestQueue t;
    t.init();
    EXPECT_FALSE(t.isFull());
}

TEST(TestQueue, InsertTest)
{
    TestQueue t;

    t.init();
    EXPECT_FALSE(t.isFull());

    TestQueue::Sender writer(t);
    writer.write(100U);
    EXPECT_EQ(t.size(), 1U);
    EXPECT_EQ(t.getStats().maxLoad, 1U);
}

TEST(TestQueue, GetInitialSizeNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;
    t.init();
    EXPECT_EQ(t.size(), 0U);
}

TEST(TestQueue, IsFullTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;
    t.init();
    EXPECT_FALSE(t.isFull());
}

TEST(TestQueue, InsertTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;

    t.init();
    EXPECT_FALSE(t.isFull());

    TestQueueNoLockSpecialization::Sender writer(t);
    writer.write(100U);
    EXPECT_EQ(t.size(), 1U);
    EXPECT_EQ(t.getStats().maxLoad, 1U);
}

TEST(TestQueue, MaxLoadTestNoLockSpecialization)
{
    TestQueueNoLockSpecialization t;

    t.init();
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

    t.init();
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

    t.init();
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

TEST(TestQueue, ExternalMutexTest)
{
    uint8_t volatile queue_mutex{
        0xFFU}; // on init method of queue this value should be initialized to 0
    TestQueueExternalMutex testingQueue{};

    // This init method should exist for this TestQueueExternalMutex type
    testingQueue.init("", &queue_mutex);
    EXPECT_EQ(queue_mutex, 0U);
    TestQueueExternalMutex::Sender writer(testingQueue);
    writer.write(100U);
}

} // namespace test
} // namespace queue
} // namespace middleware
