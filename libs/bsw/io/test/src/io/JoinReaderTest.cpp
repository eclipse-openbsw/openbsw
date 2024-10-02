// Copyright 2024 Accenture.

#include "io/JoinReader.h"

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

namespace
{
struct JoinReaderTest : ::testing::Test
{
    static size_t const QUEUE_SIZE       = 20;
    static size_t const MAX_ELEMENT_SIZE = 8;
    static size_t const NUM_QUEUES       = 8;

    using Q      = ::io::MemoryQueue<QUEUE_SIZE, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Q>;
    using Writer = ::io::MemoryQueueWriter<Q>;

    JoinReaderTest()
    {
        for (size_t i = 0; i < NUM_QUEUES; i++)
        {
            _q.emplace_back();
            _w.emplace_back(_q[i]);
            auto& r    = _r.emplace_back(_q[i]);
            readers[i] = &r;
        }
        new (_j.emplace())::io::JoinReader<NUM_QUEUES>(readers);
    }

    ::estd::vec<Q, NUM_QUEUES> _q;
    ::estd::vec<Writer, NUM_QUEUES> _w;
    ::estd::vec<Reader, NUM_QUEUES> _r;
    ::io::IReader* readers[NUM_QUEUES];
    ::estd::optional<::io::JoinReader<NUM_QUEUES>> _j;
};

size_t const JoinReaderTest::QUEUE_SIZE;
size_t const JoinReaderTest::MAX_ELEMENT_SIZE;
size_t const JoinReaderTest::NUM_QUEUES;

/**
 * \refs:   SMD_io_JoinReader
 * \desc
 * The value of maxSize() of a JoinReader should be equal to all the source readers
 */
TEST_F(JoinReaderTest, max_size_returns_sources_max_size)
{
    for (auto r : _r)
    {
        EXPECT_EQ(_j->maxSize(), r.maxSize());
    }
}

/**
 * \refs:   SMD_io_JoinReader
 * \desc
 * Release when empty doesnt't crash, even past the number of queues
 */
TEST_F(JoinReaderTest, release_empty_no_crash)
{
    for (size_t i = 0; i < NUM_QUEUES + 1; i++)
    {
        _j->release();
        _j->peek();
    }
}

/**
 * \refs:   SMD_io_JoinReader
 * \desc
 * With empty queues, an empty slice is returned
 */
TEST_F(JoinReaderTest, empty_returns_empty_slice)
{
    auto r = _j->peek();
    ASSERT_EQ(r.size(), 0);
}

/**
 * \refs:   SMD_io_JoinReader
 * \desc
 * when release isn't called, the same element keeps getting returned
 */
TEST_F(JoinReaderTest, no_release_same_element)
{
    size_t const NUM_ELTS = QUEUE_SIZE / MAX_ELEMENT_SIZE;
    for (size_t i = 0; i < NUM_ELTS; i++)
    {
        for (size_t j = 0; j < NUM_QUEUES; j++)
        {
            auto r = _w[j].allocate(MAX_ELEMENT_SIZE);
            ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
            r[0] = i;
            r[1] = j;
            _w[j].commit();
        }
    }
    auto r = _j->peek();
    ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
    ASSERT_EQ(r[0], 0);
    ASSERT_EQ(r[1], 0);
    r = _j->peek();
    ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
    ASSERT_EQ(r[0], 0);
    ASSERT_EQ(r[1], 0);
    _j->release();
    r = _j->peek();
    ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
    ASSERT_EQ(r[0], 0);
    ASSERT_EQ(r[1], 1);
    r = _j->peek();
    ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
    ASSERT_EQ(r[0], 0);
    ASSERT_EQ(r[1], 1);
}

/**
 * \refs:   SMD_io_JoinReader
 * \desc
 * With full queues, all the values are returned according to the RR algorithm
 */
TEST_F(JoinReaderTest, join_reader_full_queues)
{
    size_t const NUM_ELTS = QUEUE_SIZE / MAX_ELEMENT_SIZE;
    for (size_t i = 0; i < NUM_ELTS; i++)
    {
        for (size_t j = 0; j < NUM_QUEUES; j++)
        {
            auto r = _w[j].allocate(MAX_ELEMENT_SIZE);
            ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
            r[0] = i;
            r[1] = j;
            _w[j].commit();
        }
    }
    for (size_t i = 0; i < NUM_ELTS; i++)
    {
        for (size_t j = 0; j < NUM_QUEUES; j++)
        {
            auto r = _j->peek();
            ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
            ASSERT_EQ(r[0], i);
            ASSERT_EQ(r[1], j);
            _j->release();
        }
    }
}

/**
 * \refs:   SMD_io_JoinReader
 * \desc
 * The stats are correctly updated
 */
TEST_F(JoinReaderTest, stats_correctly_updated)
{
    size_t const NUM_ELTS = QUEUE_SIZE / MAX_ELEMENT_SIZE;
    for (size_t i = 0; i < NUM_ELTS; i++)
    {
        for (size_t j = 0; j < NUM_QUEUES - 1; j++)
        {
            auto r = _w[j].allocate(MAX_ELEMENT_SIZE);
            ASSERT_EQ(r.size(), MAX_ELEMENT_SIZE);
            r[0] = i;
            r[1] = j;
            _w[j].commit();
        }
    }
    for (size_t i = 0; i < NUM_ELTS; i++)
    {
        for (size_t j = 0; j < NUM_QUEUES - 1; j++)
        {
            auto const s = _j->peek();
            ASSERT_EQ(s.size(), MAX_ELEMENT_SIZE);
            _j->release();
        }
    }
    for (size_t j = 0; j < NUM_QUEUES; j++)
    {
        if (j == NUM_QUEUES - 1)
        {
            ASSERT_EQ(_j->stats[NUM_QUEUES - 1], 0) << "@" << j;
        }
        else
        {
            ASSERT_EQ(_j->stats[j], NUM_ELTS) << "@" << j;
        }
    }
}

} // namespace
