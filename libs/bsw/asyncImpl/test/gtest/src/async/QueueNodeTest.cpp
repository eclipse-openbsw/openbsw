// Copyright 2024 Accenture.

#include "async/QueueNode.h"

#include "async/Queue.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class TestNode : public QueueNode<TestNode>
{};

TEST(QueueNodeTest, testAll)
{
    TestNode cut;
    {
        // expect node to be not enqueued
        EXPECT_FALSE(cut.isEnqueued());
    }
    {
        // enqueue and expect it to be enqueued
        cut.enqueue();
        EXPECT_TRUE(cut.isEnqueued());
        EXPECT_TRUE(cut.getNext() == 0L);
        // dequeue it and expect it to no more being enqueued
        EXPECT_EQ(0L, cut.dequeue());
        EXPECT_FALSE(cut.isEnqueued());
    }
    {
        TestNode next;
        cut.enqueue();
        EXPECT_TRUE(cut.isEnqueued());
        EXPECT_TRUE(cut.getNext() == 0L);
        cut.setNext(&next);
        EXPECT_TRUE(cut.isEnqueued());
        EXPECT_EQ(&next, cut.getNext());
        cut.setNext(0L);
        EXPECT_TRUE(cut.isEnqueued());
        EXPECT_TRUE(cut.getNext() == 0L);
    }
    {
        TestNode next;
        cut.enqueue();
        cut.setNext(&next);
        EXPECT_TRUE(cut.isEnqueued());
        EXPECT_EQ(&next, cut.getNext());
        // dequeue and expect pointer to next
        EXPECT_EQ(&next, cut.dequeue());
        EXPECT_FALSE(cut.isEnqueued());
    }
}

} // namespace
