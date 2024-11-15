// Copyright 2024 Accenture.

#include "io/MemoryQueue.h"

#include <etl/array.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace memoryQueueExample
{

struct CanFrame
{
    uint32_t id{};
    ::etl::span<uint8_t> data{};
};

namespace spsc
{
// EXAMPLE_BEGIN IWriter

/**
 * Returns the number of bytes available in the CAN hardware buffer, zero if no frame is available.
 * The maximum rx size is 8 bytes.
 */
size_t getCanRxSize();

/**
 * Reads an available CAN frame into a provided object of type CanFrame.
 * \return true if frame was successfully read from hardware, false otherwise.
 */
bool readCanFrame(CanFrame& frame);

/**
 * Tries to read a CAN frame from the hardware and forwards it to the channel represented by a given
 * IWriter.
 * \param writer IWriter to use for sending the frame.
 * \return true if frame has successfully been forwarded, false otherwise.
 */
bool forwardCanFrame(::io::IWriter& writer)
{
    // Check if a frame is available.
    auto const size = getCanRxSize();
    if (size == 0)
    {
        // No frame available.
        return false;
    }
    // Allocate data from channel. We need room for a uint32_t id + data.
    auto data = writer.allocate(sizeof(::etl::be_uint32_t) + size);
    if (data.size() == 0)
    {
        // Couldn't allocate data.
        return false;
    }
    // The big endian 32bit id comes first in the serialization.
    etl::be_uint32_ext_t id{data.data()};
    data.advance(id.size());
    // We pass the slice data to the frame so that readCanFrame can read the data directly from
    // the hardware to the allocated memory.
    CanFrame frame;
    frame.data = data;

    if (!readCanFrame(frame))
    {
        // Reading the frame from hardware failed.
        return false;
    }
    // We need to update the previously allocated id.
    id = frame.id;
    // Commit data to channel.
    writer.commit();
    return true;
}

// EXAMPLE_END IWriter
} // namespace spsc

namespace mpsc
{
using ::memoryQueueExample::spsc::getCanRxSize;
using ::memoryQueueExample::spsc::readCanFrame;

struct Lock
{};

// EXAMPLE_BEGIN IWriter_MPSC
bool forwardCanFrame(::io::IWriter& writer)
{
    // Check if a frame is available.
    auto const size = getCanRxSize();
    if (size == 0)
    {
        // No frame available.
        return false;
    }
    // Allocate temporary CanFrame on the stack.
    uint8_t rxData[8];
    CanFrame rxFrame;
    rxFrame.data = rxData;

    // Reading takes too long to surround with lock.
    if (!readCanFrame(rxFrame))
    {
        // Reading the frame from hardware failed.
        return false;
    }
    // RAII lock to block concurrent access to writer.
    Lock lock;
    (void)lock;
    // Allocate data from channel. We need room for a uint32_t id + data.
    auto data = writer.allocate(sizeof(::etl::be_uint32_t) + size);
    if (data.size() == 0)
    {
        // Couldn't allocate data.
        return false;
    }
    // The big endian 32bit id comes first in the serialization.
    etl::be_uint32_ext_t{data.data()} = rxFrame.id;
    data.advance(sizeof(etl::be_uint32_t));
    // Copy payload to allocated data.
    ::etl::copy(rxFrame.data, data);
    // Commit data to channel.
    writer.commit();
    return true;
}

// EXAMPLE_END IWriter_MPSC
} // namespace mpsc

// EXAMPLE_BEGIN IReader
/**
 * Tries to receive a CanFrame from a given IReader.
 * \param frame CanFrame to receive to, i.e. frame provides memory to copy to.
 * \param reader IReader to try to receive a frame from.
 * \return true if a frame was read, false otherwise.
 */
bool receiveCanFrame(CanFrame& frame, ::io::IReader& reader)
{
    auto data = reader.peek();
    if (data.size() < sizeof(uint32_t))
    {
        // No frame available (A frame consists of at least uint32_t id)
        return false;
    }
    // Copy data to frame. We expect a big endian 32bit id followed by the actual data.
    frame.id = etl::be_uint32_t(data.data());
    data.advance(sizeof(etl::be_uint32_t));
    ::etl::copy(data, frame.data);
    frame.data = frame.data.first(data.size());
    // Release data to channel.
    reader.release();
    return true;
}

// EXAMPLE_END IReader

TEST(MemoryQueueExample, spsc)
{
    using spsc::forwardCanFrame;
    using DataStream       = ::io::MemoryQueue<1024, 12>;
    using DataStreamWriter = ::io::MemoryQueueWriter<DataStream>;
    using DataStreamReader = ::io::MemoryQueueReader<DataStream>;

    DataStream dataStream;

    auto dataStreamWriter = DataStreamWriter{dataStream};
    auto dataStreamReader = DataStreamReader{dataStream};

    uint8_t rxData[8];
    CanFrame rxFrame;
    rxFrame.data = rxData;

    EXPECT_FALSE(receiveCanFrame(rxFrame, dataStreamReader));

    EXPECT_TRUE(forwardCanFrame(dataStreamWriter));
    EXPECT_TRUE(receiveCanFrame(rxFrame, dataStreamReader));

    EXPECT_EQ(4, rxFrame.data.size());
    EXPECT_EQ(123, rxFrame.id);
    EXPECT_THAT(rxFrame.data, ::testing::ElementsAre(1, 2, 3, 4));
}

TEST(MemoryQueueExample, mpsc)
{
    using mpsc::forwardCanFrame;
    using DataStream       = ::io::MemoryQueue<1024, 12>;
    using DataStreamWriter = ::io::MemoryQueueWriter<DataStream>;
    using DataStreamReader = ::io::MemoryQueueReader<DataStream>;

    DataStream dataStream;

    auto dataStreamWriter = DataStreamWriter{dataStream};
    auto dataStreamReader = DataStreamReader{dataStream};

    uint8_t rxData[8];
    CanFrame rxFrame;
    rxFrame.data = rxData;

    EXPECT_FALSE(receiveCanFrame(rxFrame, dataStreamReader));

    EXPECT_TRUE(forwardCanFrame(dataStreamWriter));
    EXPECT_TRUE(receiveCanFrame(rxFrame, dataStreamReader));

    EXPECT_EQ(4, rxFrame.data.size());
    EXPECT_EQ(123, rxFrame.id);
    EXPECT_THAT(rxFrame.data, ::testing::ElementsAre(1, 2, 3, 4));
}

namespace non_virtual_api
{
// EXAMPLE_BEGIN WriterReader
/**
 * Forwards data from a reader of a MemoryQueue to a writer of another MemoryQueue
 * \param source    Reader of the MemoryQueue that acts as source of data
 * \param destination   Writer of the MemoryQueue that acts as sink of data
 * \return true if data has been forwarded, false otherwise.
 */
template<class Queue>
bool forwardData(typename Queue::Reader& source, typename Queue::Writer& destination)
{
    auto srcData = source.peek();
    if (srcData.size() == 0)
    {
        // No data available.
        return false;
    }
    // Allocate space in destination
    auto dstData = destination.allocate(srcData.size());
    if (dstData.size() == 0)
    {
        // Destination is full.
        return false;
    }
    (void)::etl::mem_copy(srcData.begin(), srcData.end(), dstData.begin());
    destination.commit();
    source.release();
    return true;
}

// EXAMPLE_END WriterReader
} // namespace non_virtual_api

TEST(MemoryQueueExample, non_virtual_interface)
{
    using non_virtual_api::forwardData;
    // EXAMPLE_BEGIN WriterReader2
    using DataStream       = ::io::MemoryQueue<1024, 12>;
    using DataStreamWriter = DataStream::Writer;
    using DataStreamReader = DataStream::Reader;

    DataStream source, destination;
    auto srcWriter = DataStreamWriter{source};
    auto srcReader = DataStreamReader{source};
    auto dstWriter = DataStreamWriter{destination};
    auto dstReader = DataStreamReader{destination};

    {
        auto d = srcWriter.allocate(4);
        ASSERT_EQ(4, d.size());
        ::etl::be_uint32_ext_t{d.data()} = 0x1234;
        srcWriter.commit();
    }

    ASSERT_TRUE(forwardData<DataStream>(srcReader, dstWriter));

    {
        auto d = dstReader.peek();
        ASSERT_EQ(4, d.size());
        ASSERT_EQ(0x1234, ::etl::be_uint32_t(d.data()));
    }
    // EXAMPLE_END WriterReader2
}

namespace virtual_api
{
// EXAMPLE_BEGIN IWriterIReader
/**
 * Forwards data from an IReader to a IWriter.
 * \param source    IReader that acts as source of data
 * \param destination   IWriter that acts as sink of data
 * \return true if data has been forwarded, false otherwise.
 */
bool forwardData(::io::IReader& source, ::io::IWriter& destination)
{
    auto srcData = source.peek();
    if (srcData.size() == 0)
    {
        // No data available.
        return false;
    }
    // Allocate space in destination
    auto dstData = destination.allocate(srcData.size());
    if (dstData.size() == 0)
    {
        // Destination is full.
        return false;
    }
    (void)::etl::copy(srcData, dstData);
    destination.commit();
    source.release();
    return true;
}

// EXAMPLE_END IWriterIReader
} // namespace virtual_api

TEST(MemoryQueueExample, virtual_interface)
{
    using virtual_api::forwardData;
    // EXAMPLE_BEGIN IWriterIReader2
    using DataStream       = ::io::MemoryQueue<1024, 12>;
    using DataStreamWriter = ::io::MemoryQueueWriter<DataStream>;
    using DataStreamReader = ::io::MemoryQueueReader<DataStream>;

    DataStream source, destination;
    auto srcWriter = DataStreamWriter{source};
    auto srcReader = DataStreamReader{source};
    auto dstWriter = DataStreamWriter{destination};
    auto dstReader = DataStreamReader{destination};

    {
        auto d = srcWriter.allocate(4);
        ASSERT_EQ(4, d.size());
        ::etl::be_uint32_ext_t{d.data()} = 0x1234;
        srcWriter.commit();
    }

    ASSERT_TRUE(forwardData(srcReader, dstWriter));

    {
        auto d = dstReader.peek();
        ASSERT_EQ(4, d.size());
        ASSERT_EQ(0x1234, ::etl::be_uint32_t(d.data()));
    }
    // EXAMPLE_END IWriterIReader2
}

// These implementations mimic the reception of a four byte CAN frame

namespace spsc
{

size_t getCanRxSize() { return 4; }

bool readCanFrame(CanFrame& frame)
{
    etl::array<uint8_t, 4> txData{1, 2, 3, 4};
    frame.id = 123;
    ::etl::copy(::etl::make_span(txData), frame.data);
    frame.data = frame.data.first(sizeof(txData));
    return true;
}
} // namespace spsc

TEST(MemoryQueueExample, maximum_wasted_memory)
{
    // EXAMPLE_BEGIN maximum_wasted_memory_1
    using Queue = ::io::MemoryQueue<21, 8>;
    Queue queue;
    ::io::MemoryQueueWriter<Queue> writer{queue};
    ::io::MemoryQueueReader<Queue> reader{queue};

    auto data = writer.allocate(7);
    writer.commit();
    // EXAMPLE_END maximum_wasted_memory_1
    ASSERT_EQ(7, data.size());
    // EXAMPLE_BEGIN maximum_wasted_memory_2
    data = writer.allocate(1);
    writer.commit();
    // EXAMPLE_END maximum_wasted_memory_2
    ASSERT_EQ(1, data.size());
    ASSERT_EQ(0, writer.available());
    // EXAMPLE_BEGIN maximum_wasted_memory_3
    data = reader.peek();
    reader.release();
    // EXAMPLE_END maximum_wasted_memory_3
    ASSERT_EQ(7, data.size());
    ASSERT_EQ(0, writer.available());
    // EXAMPLE_BEGIN maximum_wasted_memory_4
    data = reader.peek();
    reader.release();
    // EXAMPLE_END maximum_wasted_memory_4
    ASSERT_EQ(1, data.size());
    ASSERT_EQ(21, writer.available());
    // EXAMPLE_BEGIN maximum_wasted_memory_5
    data = writer.allocate(8);
    writer.commit();
    // EXAMPLE_END maximum_wasted_memory_5
    ASSERT_EQ(8, data.size());
    ASSERT_EQ(11, writer.available());
    // EXAMPLE_BEGIN maximum_wasted_memory_6
    data = writer.allocate(8);
    writer.commit();
    // EXAMPLE_END maximum_wasted_memory_6
    ASSERT_EQ(8, data.size());
    ASSERT_EQ(0, writer.available());
    // EXAMPLE_BEGIN maximum_wasted_memory_7
    data = reader.peek();
    reader.release();
    // EXAMPLE_END maximum_wasted_memory_7
    ASSERT_EQ(8, data.size());
    ASSERT_EQ(10, writer.available());
    // EXAMPLE_BEGIN maximum_wasted_memory_8
    data = reader.peek();
    reader.release();
    // EXAMPLE_END maximum_wasted_memory_8
    ASSERT_EQ(8, data.size());
    ASSERT_EQ(21, writer.available());
}

} // namespace memoryQueueExample
