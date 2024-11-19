// Copyright 2024 Accenture.

#include "io/MemoryQueue.h"
#include "io/SplitWriter.h"

#include <gmock/gmock.h>

namespace splitWriterExample
{

// EXAMPLE_BEGIN SplitWriter
/**
 * Forwards all data from an IReader to an IWriter basically connecting them.
 * \param source    IReader acting as source of the connection.
 * \param destination    IWriter acting as destination of the connection.
 *
 * This function will forward one slice of data when being called. If data is available
 * from the source but no space is available in the destination, the source data will be kept
 * and retried when the function is called the next time.
 */
void forwardData(::io::IReader& source, ::io::IWriter& destination)
{
    auto const srcData = source.peek();
    if (srcData.size() > 0)
    {
        auto dstData = destination.allocate(srcData.size());
        if (dstData.size() >= srcData.size())
        {
            ::estd::memory::copy(dstData, srcData);
            destination.commit();
            // Only release data after successful forwarding.
            source.release();
        }
    }
}

/**
 * This usage example demonstrates how to forward data from one reader to two writers. It
 * focuses on the setup and forwarding not on how the actual data is put into the queues or
 * read from it.
 */
TEST(SplitWriter, UsageExample)
{
    using Queue = ::io::MemoryQueue<1024, 16>;
    // Create two output queues.
    Queue q1;
    Queue q2;
    io::MemoryQueueWriter<Queue> w1{q1};
    io::MemoryQueueWriter<Queue> w2{q2};
    ::io::IWriter* writers[2] = {&w1, &w2};
    ::io::SplitWriter<2> w{writers};

    // Create input queue
    Queue q3;
    io::MemoryQueueReader<Queue> r3{q3};

    // ...

    // This function can be called cyclically to forward data from q3 to q1 and q2.
    forwardData(r3, w);
}

// EXAMPLE_END SplitWriter
} // namespace splitWriterExample
