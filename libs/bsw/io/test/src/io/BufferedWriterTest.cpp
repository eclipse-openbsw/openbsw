// Copyright 2024 Accenture.

#include "io/BufferedWriter.h"

#include "io/MemoryQueue.h"

#include <gmock/gmock.h>

#include <cstddef>
#include <cstdint>

namespace
{
struct BufferedWriterTest : ::testing::Test
{
    static size_t const QUEUE_SIZE       = 10 * 1024;
    static size_t const MAX_ELEMENT_SIZE = 1024;

    using Q = ::io::MemoryQueue<QUEUE_SIZE, MAX_ELEMENT_SIZE>;

    BufferedWriterTest() : _q(), _w(_q), _mqw(_q), _mqr(_q), _bmqw(_mqw) {}

    Q _q;
    Q::Writer _w;
    ::io::MemoryQueueWriter<Q> _mqw;
    ::io::MemoryQueueReader<Q> _mqr;
    ::io::BufferedWriter _bmqw;
};

size_t const BufferedWriterTest::QUEUE_SIZE;
size_t const BufferedWriterTest::MAX_ELEMENT_SIZE;

/**
 * \refs:   SMD_io_BufferedWriter
 * \desc
 * The value of maxSize() of a BufferedWriterTest should be equal to the one of the
 * used destination IWriter.
 */
TEST_F(BufferedWriterTest, max_size_returns_destinations_max_size)
{
    EXPECT_EQ(_w.maxSize(), _bmqw.maxSize());
}

/**
 * \refs:   SMD_io_BufferedWriter
 * \desc
 * Trying to allocate more than maxSize bytes will return an empty slice.
 */
TEST_F(BufferedWriterTest, allocate_exceeding_max_size_will_return_empty_slice)
{
    auto const b = _bmqw.allocate(_bmqw.maxSize() + 1);
    EXPECT_EQ(0, b.size());
}

/**
 * \refs:   SMD_io_BufferedWriter
 * \desc
 * Trying to allocate maxSize bytes will return a slice of size maxSize.
 */
TEST_F(BufferedWriterTest, allocate_max_size_will_return_max_size_slice)
{
    auto const b = _bmqw.allocate(_bmqw.maxSize());
    EXPECT_EQ(_bmqw.maxSize(), b.size());
}

/**
 * \refs:   SMD_io_BufferedWriter
 * \desc
 * When allocating memory on a full writer, it shall return an empty slice.
 */
TEST_F(BufferedWriterTest, allocate_on_full_writer_will_return_empty_slice)
{
    for (size_t i = 0; i < 10 * 10; ++i)
    {
        auto const b = _bmqw.allocate(100);
        ASSERT_EQ(100, b.size()) << "@" << i;
        _bmqw.commit();
    }
    _bmqw.flush();
    auto const b = _bmqw.allocate(100);
    ASSERT_EQ(0, b.size());
}

/**
 * \refs:   SMD_io_BufferedWriter
 * \desc
 * Calling commit() on a BufferedWriter before allocating something shall have no effect on the
 * destination IWriter of the BufferedWriter.
 */
TEST_F(BufferedWriterTest, commit_without_allocation_has_no_effect)
{
    EXPECT_EQ(QUEUE_SIZE, _w.available());
    _bmqw.commit();
    EXPECT_EQ(QUEUE_SIZE, _w.available());
}

/**
 * \refs:   SMD_io_BufferedWriter
 * \desc
 * Writing destination.maxSize triggers a flush of the buffer when allocating the
 * next byte.
 */
TEST_F(BufferedWriterTest, trigger_buffer_flush)
{
    for (size_t i = 0; i < _bmqw.maxSize(); ++i)
    {
        auto const b = _bmqw.allocate(1);
        ASSERT_EQ(1, b.size());
        _bmqw.commit();
    }
    ASSERT_EQ(0, _mqr.peek().size());
    auto const b = _bmqw.allocate(1);
    ASSERT_EQ(1, b.size());
    EXPECT_EQ(_mqr.peek().size(), _bmqw.maxSize());
}

/**
 * \refs:   SMD_io_BufferedWriter
 * \example
 * This examples shows how to transmit 2000 single bytes through a buffered writer.
 */
TEST_F(BufferedWriterTest, transmit_2000_bytes)
{
    for (size_t i = 0; i < 2000; ++i)
    {
        auto b = _bmqw.allocate(1);
        ASSERT_EQ(1, b.size());
        b[0] = static_cast<uint8_t>(i);
        _bmqw.commit();
    }
    _bmqw.flush();
    size_t transmitted    = 0;
    uint8_t expectedValue = 0;
    while (_mqr.peek().size() != 0)
    {
        auto const b = _mqr.peek();
        for (auto v : b)
        {
            EXPECT_EQ(expectedValue, v);
            ++expectedValue;
        }
        transmitted += b.size();
        _mqr.release();
    }
    EXPECT_EQ(2000, transmitted);
}

} // namespace
