// Copyright 2024 Accenture.

#include "io/ForwardingReader.h"
#include "io/MemoryQueue.h"

#include <gmock/gmock.h>

namespace forwardingReaderExample
{

// EXAMPLE_BEGIN ForwardingReader

/**
 * Function placeholder for consuming data from a given reader.
 */
void consumeData(::io::IReader& reader);

/**
 * This usage example demonstrates the usage of ::io::ForwardingReader. It's very high level but
 * gives an idea about possible use cases.
 */
TEST(ForwardingReader, UsageExample)
{
    using Queue = ::io::MemoryQueue<1024, 16>;
    // Create one input queue.
    Queue input;
    // Create one queue to mirror read data to a debug device.
    Queue debugOutput;
    io::MemoryQueueReader<Queue> inputReader{input};
    io::MemoryQueueWriter<Queue> debugWriter{debugOutput};
    // Create the ForwardingReader.
    ::io::ForwardingReader r{inputReader, debugWriter};

    // This function can be called cyclically to process data from the input queue. The consumed
    // data is also copied to the debugOutput queue.
    consumeData(r);
}

// EXAMPLE_END ForwardingReader

void consumeData(::io::IReader& reader)
{
    auto d = reader.peek();
    if (d.size() > 0)
    {
        // do sth. with the data...

        reader.release();
    }
}
} // namespace forwardingReaderExample
