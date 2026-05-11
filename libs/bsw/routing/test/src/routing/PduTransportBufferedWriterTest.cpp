/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportBufferedWriter.h"

#include "io/MemoryQueue.h"

#include <bsp/timer/SystemTimerMock.h>

#include <gmock/gmock.h>

using namespace ::testing;

namespace
{
struct PduTransportBufferedWriterTest : ::testing::Test
{
    static size_t const MAX_ELEMENT_SIZE = 1416U;
    static size_t const QUEUE_SIZE       = 3 * (MAX_ELEMENT_SIZE + sizeof(uint16_t));

    using SystemTimer = NiceMock<SystemTimerMock>;
    using Q           = ::io::MemoryQueue<QUEUE_SIZE, MAX_ELEMENT_SIZE>;

    PduTransportBufferedWriterTest()
    : _q(), _w(_q), _mqw(_q), _mqr(_q), _bmqw(_mqw, 1U), _timestamp(0)
    {
        ON_CALL(_systemTimerMock, getSystemTimeUs32Bit())
            .WillByDefault([&t = _timestamp] { return t; });
    }

    Q _q;
    Q::Writer _w;
    ::io::MemoryQueueWriter<Q> _mqw;
    ::io::MemoryQueueReader<Q> _mqr;

    ::routing::PduTransportBufferedWriter _bmqw;

    uint32_t _timestamp;
    SystemTimer _systemTimerMock;
};

size_t const PduTransportBufferedWriterTest::MAX_ELEMENT_SIZE;
size_t const PduTransportBufferedWriterTest::QUEUE_SIZE;

/**
 * \desc
 * The value of max_size() of a PduTransportBufferedWriterTest should be equal to the one of the
 * used destination IWriter.
 */
TEST_F(PduTransportBufferedWriterTest, max_size_returns_destinations_max_size)
{
    EXPECT_EQ(_w.maxSize(), _bmqw.maxSize());
}

/**
 * \desc
 * Trying to allocate more than maxSize bytes will return an empty span.
 */
TEST_F(PduTransportBufferedWriterTest, allocate_exceeding_max_size_will_return_empty_span)
{
    auto const b = _bmqw.allocate(_bmqw.maxSize() + 1);
    EXPECT_EQ(0, b.size());
}

/**
 * \desc
 * Trying to allocate maxSize bytes will return a span of size maxSize.
 */
TEST_F(PduTransportBufferedWriterTest, allocate_max_size_will_return_max_size_span)
{
    auto const b = _bmqw.allocate(_bmqw.maxSize());
    EXPECT_EQ(_bmqw.maxSize(), b.size());
}

/**
 * \desc
 * When allocating memory on a full writer, it shall return an empty span.
 */
TEST_F(PduTransportBufferedWriterTest, allocate_on_full_writer_will_return_empty_span)
{
    constexpr size_t ELEMENT_SIZE = 10;
    // the number of elements we can buffer is equal to three times the number of elements that fit
    // entirely in one frame. As such, the parenthesis are required here.
    for (size_t i = 0; i < 3 * (_bmqw.maxSize() / ELEMENT_SIZE); ++i)
    {
        auto const b = _bmqw.allocate(ELEMENT_SIZE);
        ASSERT_EQ(ELEMENT_SIZE, b.size());
        _bmqw.commit();
    }
    _bmqw.flush();

    auto const b = _bmqw.allocate(ELEMENT_SIZE);
    ASSERT_EQ(0, b.size());
}

/**
 * \desc
 * Calling commit() on a PduTransportBufferedWriter before allocating something shall have no effect
 * on the destination IWriter of the PduTransportBufferedWriter.
 */
TEST_F(PduTransportBufferedWriterTest, commit_without_allocation_has_no_effect)
{
    EXPECT_EQ(QUEUE_SIZE, _w.available());
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(0, _bmqw.flushes());
    _bmqw.commit();
    EXPECT_EQ(QUEUE_SIZE, _w.available());
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(0, _bmqw.flushes());
}

/**
 * \desc
 * Writing destination.maxSize triggers a flush of the buffer when allocating the
 * next byte.
 */
TEST_F(PduTransportBufferedWriterTest, trigger_buffer_flush)
{
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(0, _bmqw.flushes());

    for (size_t i = 0; i < _bmqw.maxSize(); i++)
    {
        auto const b = _bmqw.allocate(1);
        ASSERT_EQ(1, b.size());
        _bmqw.commit();
    }
    ASSERT_EQ(0, _mqr.peek().size());
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(0, _bmqw.flushes());

    auto const b = _bmqw.allocate(1);
    ASSERT_EQ(1, b.size());
    EXPECT_EQ(_bmqw.maxSize(), _mqr.peek().size());
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(1, _bmqw.flushes());
}

/**
 * \example
 * This examples shows howto transmit 2000 single bytes through a buffered writer.
 */
TEST_F(PduTransportBufferedWriterTest, transmit_2000_bytes)
{
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(0, _bmqw.flushes());
    for (size_t i = 0; i < 2000; ++i)
    {
        auto b = _bmqw.allocate(1);
        ASSERT_EQ(1, b.size());
        b[0] = static_cast<uint8_t>(i);
        _bmqw.commit();
    }
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(1, _bmqw.flushes());
    _bmqw.flush();
    EXPECT_EQ(0, _bmqw.timeoutSends());
    EXPECT_EQ(2, _bmqw.flushes());
    size_t transmitted    = 0;
    uint8_t expectedValue = 0;
    while (!_mqr.peek().empty())
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

/**
 * \desc
 * Flushing the current buffer updates the timestamp.
 */
TEST_F(PduTransportBufferedWriterTest, flush_updates_timestamp)
{
    uint32_t timeouts[] = {1, 2, 3, 5, 10};

    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);
        _timestamp = 0;

        EXPECT_EQ(0, _mqr.peek().size());

        w.allocate(1);
        w.commit();
        w.flush();

        EXPECT_EQ(1, _mqr.peek().size());
        _mqr.release();

        _timestamp = timeout * 1000U;

        w.checkTransmissionTimeout();

        EXPECT_EQ(0, _mqr.peek().size());
    }
}

/**
 * \desc
 * If the transmission timeout is equal to zero, commiting data also flushes the buffer.
 */
TEST_F(PduTransportBufferedWriterTest, commit_triggers_flush_zero_transmission_timeout)
{
    ::routing::PduTransportBufferedWriter w(_mqw, 0U);

    w.checkTransmissionTimeout();

    EXPECT_EQ(0, _mqr.peek().size());

    w.allocate(1);
    w.commit();

    EXPECT_EQ(1, w.timeoutSends());
    EXPECT_EQ(1, w.flushes());
    EXPECT_EQ(1, _mqr.peek().size());
    _mqr.release();

    w.checkTransmissionTimeout();

    EXPECT_EQ(1, w.timeoutSends());
    EXPECT_EQ(1, w.flushes());
    EXPECT_EQ(0, _mqr.peek().size());
}

/**
 * \desc
 * The expiration of the transmission timeout triggers a flush of the current buffer.
 */
TEST_F(PduTransportBufferedWriterTest, expiration_of_transmission_timeout_triggers_flush)
{
    constexpr uint32_t T = 10;
    uint32_t timeouts[]  = {1, 2, 3, 5, 10};

    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);

        _timestamp = 0;
        for (uint32_t i = 0; i < T; ++i, _timestamp = 1000U * i)
        {
            w.checkTransmissionTimeout();

            if ((i == 0) || (i % timeout != 0))
            {
                EXPECT_EQ(0U, _mqr.peek().size());
            }
            else
            {
                EXPECT_EQ(timeout, _mqr.peek().size());
                _mqr.release();
            }

            w.allocate(1);
            w.commit();
        }
        uint32_t sends = (T - 1) / timeout;
        EXPECT_EQ(sends, w.timeoutSends());
        EXPECT_EQ(sends, w.flushes());
    }
}

/**
 * \desc
 * if the transmission timeout will expire in under 1ms, the current buffer is flushed.
 */
TEST_F(
    PduTransportBufferedWriterTest, expiration_of_transmission_timeout_in_under_1ms_triggers_flush)
{
    constexpr uint32_t T = 10;
    uint32_t timeouts[]  = {1, 2, 3, 5, 10};

    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);

        for (uint32_t i = 0; i < T; ++i)
        {
            if ((i == 0) || (i % timeout != 0))
            {
                _timestamp = 1000U * i;
                w.checkTransmissionTimeout();
                EXPECT_EQ(0U, _mqr.peek().size());
            }
            else
            {
                _timestamp += 1U;
                w.checkTransmissionTimeout();
                EXPECT_EQ(timeout, _mqr.peek().size());
                _mqr.release();
                _timestamp = 1000U * i;
            }

            w.allocate(1);
            w.commit();
        }
        uint32_t sends = (T - 1) / timeout;
        EXPECT_EQ(sends, w.timeoutSends());
        EXPECT_EQ(sends, w.flushes());
    }
}

/**
 * \desc
 * Reset discards previously allocated data.
 */
TEST_F(PduTransportBufferedWriterTest, reset_discards_allocated_data)
{
    uint32_t timeouts[] = {0, 1, 2, 3, 5, 10};
    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);
        _timestamp = 0;

        w.allocate(1);

        w.reset();

        w.commit();
        w.flush();

        EXPECT_EQ(0, w.timeoutSends());
        EXPECT_EQ(0, w.flushes());
        EXPECT_EQ(0, _mqr.peek().size());
    }
}

/**
 * \desc
 * Reset updates the timestamp and discards previously committed data.
 */
TEST_F(PduTransportBufferedWriterTest, reset_updates_timestamp_and_discards_committed_data)
{
    uint32_t timeouts[] = {1, 2, 3, 5, 10};
    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);
        _timestamp = 0;

        w.allocate(1);
        w.commit();

        w.reset();

        _timestamp = timeout * 1000U;

        w.checkTransmissionTimeout();

        EXPECT_EQ(0, _mqr.peek().size());

        w.allocate(1);
        w.commit();

        w.reset();

        w.flush();

        EXPECT_EQ(0, _mqr.peek().size());
    }
}

/**
 * \desc
 * Flush still commits data if the timestamp is equal to its maximum value.
 */
TEST_F(PduTransportBufferedWriterTest, flush_max_timestamp)
{
    uint32_t timeouts[] = {1, 2, 3, 5, 10};
    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);
        _timestamp = std::numeric_limits<uint32_t>::max();

        w.allocate(1);
        w.commit();

        _timestamp = timeout * 1000U;

        w.flush();

        EXPECT_EQ(1, _mqr.peek().size());
        _mqr.release();
    }
}

/**
 * \desc
 * Transmission timeout still triggers a flush if the timestamp is equal to its maximum value.
 */
TEST_F(PduTransportBufferedWriterTest, transmission_timeout_max_timestamp)
{
    uint32_t timeouts[] = {1, 2, 3, 5, 10};
    for (auto const timeout : timeouts)
    {
        ::routing::PduTransportBufferedWriter w(_mqw, timeout);
        _timestamp = std::numeric_limits<uint32_t>::max();

        w.allocate(1);
        w.commit();

        _timestamp = timeout * 1000U;

        w.checkTransmissionTimeout();

        EXPECT_EQ(1, _mqr.peek().size());
        _mqr.release();
    }
}

} // namespace
