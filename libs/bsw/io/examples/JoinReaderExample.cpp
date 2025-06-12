// Copyright 2024 Accenture.

#include "io/JoinReader.h"
#include "io/MemoryQueue.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace joinReaderExample
{

// EXAMPLE_BEGIN JoinReader
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
            ::etl::copy(srcData, dstData);
            destination.commit();
            // Only release data after successful forwarding.
            source.release();
        }
    }
}

/**
 * This usage example demonstrates how to forward data from two readers to one writer. It
 * focuses on the setup and forwarding not on how the actual data is put into the queues or
 * read from it.
 */
TEST(JoinReader, UsageExample)
{
    using Queue = ::io::MemoryQueue<1024, 16>;
    // Create two input queues.
    Queue q1;
    Queue q2;
    io::MemoryQueueReader<Queue> r1{q1};
    io::MemoryQueueReader<Queue> r2{q2};
    ::io::IReader* readers[2] = {&r1, &r2};
    ::io::JoinReader<2> r{readers};

    // Create output queue
    Queue q3;
    io::MemoryQueueWriter<Queue> w3{q3};

    // ...

    // This function can be called cyclically to forward data from q1 and q2 to q3.
    forwardData(r, w3);
}

// EXAMPLE_END JoinReader
} // namespace joinReaderExample
