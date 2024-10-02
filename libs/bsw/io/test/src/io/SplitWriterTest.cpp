// Copyright 2024 Accenture.

#include "io/SplitWriter.h"

#include "io/MemoryQueue.h"

#include <estd/big_endian.h>
#include <estd/memory.h>
#include <estd/optional.h>
#include <estd/slice.h>
#include <estd/vec.h>

#include <gmock/gmock.h>

#include <cstddef>
#include <cstdint>

using namespace ::testing;

constexpr size_t QUEUE_SIZE       = 20;
constexpr size_t MAX_ELEMENT_SIZE = 8;
constexpr size_t NUM_QUEUES       = 8;

namespace
{
struct SplitWriterTest : ::testing::Test
{
    using Q      = ::io::MemoryQueue<QUEUE_SIZE, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Q>;
    using Writer = ::io::MemoryQueueWriter<Q>;

    SplitWriterTest()
    {
        for (size_t i = 0; i < NUM_QUEUES; i++)
        {
            _q.emplace_back();
            _r.emplace_back(_q[i]);
            auto& w    = _w.emplace_back(_q[i]);
            writers[i] = &w;
        }
        new (_s.emplace())::io::SplitWriter<NUM_QUEUES>(writers);
    }

    ::estd::vec<Q, NUM_QUEUES> _q;
    ::estd::vec<Writer, NUM_QUEUES> _w;
    ::estd::vec<Reader, NUM_QUEUES> _r;
    ::io::IWriter* writers[NUM_QUEUES];
    ::estd::optional<::io::SplitWriter<NUM_QUEUES>> _s;
};

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * The constructor should throw an assert if initialised with a slice that is including a nullptr.
 */
TEST_F(SplitWriterTest, cannot_initialise_with_nullptrs)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::array<::io::IWriter*, 1> array1 = {nullptr};

    ASSERT_THROW(::io::SplitWriter<1> splitWriter1(array1), ::estd::assert_exception);

    ::estd::array<::io::IWriter*, NUM_QUEUES> array_NUM_QUEUES;
    for (size_t i = 0; i < _w.size() - 1; i++)
    {
        array_NUM_QUEUES[i] = &_w[i];
    }
    array_NUM_QUEUES[_w.size() - 1] = nullptr;

    // No throw.
    ::io::SplitWriter<NUM_QUEUES - 1> splitWriter2(array_NUM_QUEUES);

    ASSERT_THROW(
        ::io::SplitWriter<NUM_QUEUES> splitWriter3(array_NUM_QUEUES), ::estd::assert_exception);
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * The constructor should throw an assert if initialised with any writer whose maxSize() does not
 * equal to all the other writers.
 */
TEST_F(SplitWriterTest, cannot_initialise_with_different_sized_writers)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    using AnotherQueue = ::io::MemoryQueue<2, 1, uint8_t>;
    AnotherQueue anotherQueue;
    ::io::MemoryQueueWriter<AnotherQueue> anotherSizedIWriter(anotherQueue);

    ::estd::array<::io::IWriter*, NUM_QUEUES> array_NUM_QUEUES;
    for (size_t i = 0; i < _w.size() - 1; i++)
    {
        array_NUM_QUEUES[i] = &_w[i];
    }
    array_NUM_QUEUES[_w.size() - 1] = &anotherSizedIWriter;

    // No throw.
    ::io::SplitWriter<NUM_QUEUES - 1> splitWriter2(array_NUM_QUEUES);

    ASSERT_THROW(
        ::io::SplitWriter<NUM_QUEUES> splitWriter3(array_NUM_QUEUES), ::estd::assert_exception);
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * The value of maxSize() of a BufferedWriterTest should be equal to all the destination writers
 */
TEST_F(SplitWriterTest, max_size_returns_destinations_max_size)
{
    for (auto w : _w)
    {
        EXPECT_EQ(_s->maxSize(), w.maxSize());
    }
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * Trying to allocate more than maxSize bytes will return an empty slice.
 */
TEST_F(SplitWriterTest, allocate_exceeding_max_size_will_return_empty_slice)
{
    auto const b = _s->allocate(_s->maxSize() + 1);
    EXPECT_EQ(0, b.size());
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * Trying to allocate maxSize bytes will return a slice of size maxSize.
 */
TEST_F(SplitWriterTest, allocate_max_size_will_return_max_size_slice)
{
    auto const b = _s->allocate(_s->maxSize());
    EXPECT_EQ(_s->maxSize(), b.size());
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * When allocating memory on a full writer, it shall return an empty slice.
 */
TEST_F(SplitWriterTest, allocate_on_full_writer_will_return_empty_slice)
{
    for (size_t i = 0; i < QUEUE_SIZE / MAX_ELEMENT_SIZE; ++i)
    {
        auto const b = _s->allocate(MAX_ELEMENT_SIZE);
        ASSERT_EQ(MAX_ELEMENT_SIZE, b.size()) << "@" << i;
        _s->commit();
    }
    auto const b = _s->allocate(MAX_ELEMENT_SIZE);
    ASSERT_EQ(0, b.size());
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * Writing to the one writer transmits data to all the readers
 */
TEST_F(SplitWriterTest, write_to_all)
{
    uint8_t const payload[] = {1, 2, 3, 4, 5};
    auto const b            = _s->allocate(sizeof(payload));
    ASSERT_EQ(sizeof(payload), b.size());
    ::estd::memory::copy(b, payload);
    _s->commit();

    for (size_t i = 0; i < _r.size(); i++)
    {
        auto r = _r[i].peek();
        ASSERT_THAT(r, ElementsAreArray(payload));
        _r[i].release();
    }
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * Re-allocating gives us the same piece of memory, even if a queue frees in between
 */
TEST_F(SplitWriterTest, reallocation_gives_same_memory)
{
    // fill all
    for (size_t i = 0; i < QUEUE_SIZE / MAX_ELEMENT_SIZE; ++i)
    {
        auto const b = _s->allocate(MAX_ELEMENT_SIZE);
        ASSERT_EQ(MAX_ELEMENT_SIZE, b.size()) << "@" << i;
        _s->commit();
    }
    auto b = _s->allocate(MAX_ELEMENT_SIZE);
    ASSERT_EQ(0, b.size());

    // free the last
    auto r = _r[NUM_QUEUES - 1].peek();
    ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
    _r[NUM_QUEUES - 1].release();

    // allocate and fill with pattern
    uint8_t const payload[] = {1, 2, 3, 4, 5};
    b                       = _s->allocate(sizeof(payload));
    ASSERT_EQ(sizeof(payload), b.size());
    ::estd::memory::copy(b, payload);

    // free the second to last
    r = _r[NUM_QUEUES - 2].peek();
    ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
    _r[NUM_QUEUES - 2].release();

    // re-allocate and check pattern
    b = _s->allocate(sizeof(payload));
    ASSERT_THAT(b, ElementsAreArray(payload));
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * When only one writer is free, the allocation is successful
 */
TEST_F(SplitWriterTest, allocate_with_one_free_works)
{
    constexpr size_t UNBLOCKED = NUM_QUEUES - 1;

    // fill everything
    for (size_t i = 0; i < QUEUE_SIZE / MAX_ELEMENT_SIZE; ++i)
    {
        auto const b = _s->allocate(MAX_ELEMENT_SIZE);
        ASSERT_EQ(MAX_ELEMENT_SIZE, b.size()) << "@" << i;
        _s->commit();
    }
    auto b = _s->allocate(MAX_ELEMENT_SIZE);
    ASSERT_EQ(0, b.size());

    // read from one
    b = _r[UNBLOCKED].peek();
    ASSERT_EQ(b.size(), MAX_ELEMENT_SIZE);
    _r[UNBLOCKED].release();

    // write works again
    b = _s->allocate(MAX_ELEMENT_SIZE);
    ASSERT_EQ(MAX_ELEMENT_SIZE, b.size());
    _s->commit();

    // drops all around
    for (size_t i = 0; i < UNBLOCKED; i++)
    {
        ASSERT_EQ(_s->drops[i], 1) << "@" << i;
        ASSERT_EQ(_s->sent[i], QUEUE_SIZE / MAX_ELEMENT_SIZE) << "@" << i;
    }
    ASSERT_EQ(_s->drops[UNBLOCKED], 0);
    ASSERT_EQ(_s->sent[UNBLOCKED], 1 + QUEUE_SIZE / MAX_ELEMENT_SIZE);
}

/**
 * \refs:   SMD_io_SplitWriter
 * \desc
 * This test case verifies that calling commit() without a previous successful allocation does
 * not try to push any data to the destinations of a SplitWriter. Having called commit() once
 * invalidates to previous allocation.
 */
TEST_F(SplitWriterTest, commit_without_previous_allocation_must_not_affect_writers)
{
    auto data = _s->allocate(4);
    ASSERT_EQ(4, data.size());
    data[0] = 0x11;
    data[1] = 0x22;
    data[2] = 0x33;
    data[3] = 0x44;
    _s->commit();
    // Check that data is copied to all readers.
    for (auto& reader : _r)
    {
        auto const d = reader.peek();
        EXPECT_EQ(4, d.size());
        EXPECT_THAT(d, ElementsAre(0x11, 0x22, 0x33, 0x44));
        // Release the data.
        reader.release();
    }
    // Now check that all readers are empty again.
    for (auto& reader : _r)
    {
        EXPECT_EQ(0, reader.peek().size());
    }
    _s->commit();
    for (auto& reader : _r)
    {
        EXPECT_EQ(0, reader.peek().size());
    }
}

} // namespace
