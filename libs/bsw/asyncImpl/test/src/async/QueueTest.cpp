// Copyright 2024 Accenture.

#include "async/Queue.h"

#include "async/QueueNode.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::async;
using namespace ::testing;

class TestNode : public QueueNode<TestNode>
{};

TEST(QueueTest, testAll)
{
    Queue<TestNode> cut;
    TestNode node1;
    TestNode node2;
    TestNode node3;
    {
        // expect empty queue on beginning
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // enqueue single node and expect it to be returned on dequeue
        cut.enqueue(node1);
        EXPECT_TRUE(node1.isEnqueued());
        EXPECT_EQ(&node1, cut.dequeue());
        EXPECT_FALSE(node1.isEnqueued());
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // enqueue multiple nodes and expect them to be dequeued in correct order
        cut.enqueue(node1);
        EXPECT_TRUE(node1.isEnqueued());
        cut.enqueue(node2);
        EXPECT_TRUE(node2.isEnqueued());
        EXPECT_EQ(&node1, cut.dequeue());
        EXPECT_FALSE(node1.isEnqueued());
        cut.enqueue(node3);
        EXPECT_TRUE(node3.isEnqueued());
        EXPECT_EQ(&node2, cut.dequeue());
        EXPECT_FALSE(node2.isEnqueued());
        EXPECT_EQ(&node3, cut.dequeue());
        EXPECT_FALSE(node3.isEnqueued());
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // enqueue and expect all nodes to be cleared
        cut.enqueue(node1);
        EXPECT_TRUE(node1.isEnqueued());
        cut.enqueue(node2);
        EXPECT_TRUE(node2.isEnqueued());
        cut.clear();
        EXPECT_FALSE(node1.isEnqueued());
        EXPECT_FALSE(node2.isEnqueued());
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // remove first node
        cut.enqueue(node1);
        EXPECT_TRUE(node1.isEnqueued());
        cut.enqueue(node2);
        EXPECT_TRUE(node2.isEnqueued());
        cut.remove(node1);
        EXPECT_FALSE(node1.isEnqueued());
        EXPECT_TRUE(node2.isEnqueued());
        EXPECT_EQ(&node2, cut.dequeue());
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // remove last node
        cut.enqueue(node1);
        EXPECT_TRUE(node1.isEnqueued());
        cut.enqueue(node2);
        EXPECT_TRUE(node2.isEnqueued());
        cut.remove(node2);
        EXPECT_TRUE(node1.isEnqueued());
        EXPECT_FALSE(node2.isEnqueued());
        EXPECT_EQ(&node1, cut.dequeue());
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // remove inner node
        cut.enqueue(node1);
        EXPECT_TRUE(node1.isEnqueued());
        cut.enqueue(node2);
        EXPECT_TRUE(node2.isEnqueued());
        cut.enqueue(node3);
        EXPECT_TRUE(node3.isEnqueued());
        cut.remove(node2);
        EXPECT_TRUE(node1.isEnqueued());
        EXPECT_FALSE(node2.isEnqueued());
        EXPECT_TRUE(node3.isEnqueued());
        EXPECT_EQ(&node1, cut.dequeue());
        EXPECT_EQ(&node3, cut.dequeue());
        EXPECT_TRUE(cut.dequeue() == 0L);
    }
    {
        // remove node that is not enqueued
        cut.remove(node1);
    }
}

} // namespace
