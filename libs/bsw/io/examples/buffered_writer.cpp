// Copyright 2024 Accenture.

#include "io/BufferedWriter.h"
#include "io/MemoryQueue.h"

#include <gmock/gmock.h>

namespace bufferedWriterExample
{

// EXAMPLE_BEGIN BufferedWriter

class CanFrameForwarder
{
    ::io::IReader& _canFrameInput;
    ::io::BufferedWriter _udpOutput;

public:
    CanFrameForwarder(::io::IReader& canFrameInput, ::io::IWriter& udpOutput)
    : _canFrameInput(canFrameInput), _udpOutput(udpOutput)
    {}

    void operator()()
    {
        // Input format is: uint32_t id, payload.
        ::estd::slice<uint8_t const> frame = _canFrameInput.peek();
        while (frame.size() > 0)
        {
            // Output format is uint8_t size, uint32_t id, payload
            ::estd::slice<uint8_t> dst = _udpOutput.allocate(frame.size() + 1);
            if (dst.size() == 0)
            {
                break;
            }
            // Format: uint8_t payload length, uint32_t id, payload
            ::estd::memory::take<uint8_t>(dst) = frame.size();
            ::estd::memory::copy(dst, frame);
            _udpOutput.commit();
            _canFrameInput.release();
            frame = _canFrameInput.peek();
        }
        _udpOutput.flush();
    }
};

/**
 * This usage example demonstrates how to read small input slices of bytes and accumulate them,
 * using a BufferedWriter into a writer that support much larger allocations.
 */
TEST(BufferedWriter, UsageExample)
{
    using CanQueue = ::io::MemoryQueue<1024, 12>;
    using UdpQueue = ::io::MemoryQueue<10240, 1400>;
    // Create output channel.
    UdpQueue outputQueue;
    ::io::MemoryQueueWriter<UdpQueue> output{outputQueue};
    ::io::BufferedWriter udpWriter{output};
    // Create input channel.
    CanQueue inputQueue;
    ::io::MemoryQueueReader<CanQueue> input{inputQueue};

    CanFrameForwarder canFrameForwarder{input, udpWriter};

    // Call this cyclically
    canFrameForwarder();
}

// EXAMPLE_END BufferedWriter
} // namespace bufferedWriterExample
