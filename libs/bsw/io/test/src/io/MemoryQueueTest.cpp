// Copyright 2024 Accenture.

#include "io/MemoryQueue.h"

#include <estd/big_endian.h>
#include <estd/memory.h>
#include <estd/slice.h>

#include <gmock/gmock.h>

#include <cstddef>
#include <cstdint>

using namespace ::testing;

namespace
{
struct MemoryQueueTest : ::testing::Test
{
    static size_t const QUEUE_SIZE       = 20;
    static size_t const MAX_ELEMENT_SIZE = 8;

    using Q = ::io::MemoryQueue<QUEUE_SIZE, MAX_ELEMENT_SIZE>;

    MemoryQueueTest() : _q(), _w(_q), _r(_q) {}

    Q _q;
    Q::Writer _w;
    Q::Reader _r;
};

size_t const MemoryQueueTest::QUEUE_SIZE;
size_t const MemoryQueueTest::MAX_ELEMENT_SIZE;

template<class Q>
struct MemoryQueueTypedTest : public ::testing::Test
{
    static size_t const QUEUE_SIZE       = Q::capacity();
    static size_t const MAX_ELEMENT_SIZE = Q::maxElementSize();
    static size_t const NUM_POSSIBLE_MAX_ELEMENT_SIZE_ALLOCATIONS
        = (QUEUE_SIZE / (MAX_ELEMENT_SIZE + sizeof(typename Q::size_type)));

    MemoryQueueTypedTest() : _q(), _w(_q), _r(_q) {}

    Q _q;
    typename Q::Writer _w;
    typename Q::Reader _r;
};

// clang-format off
template<class Q>
size_t const MemoryQueueTypedTest<Q>::QUEUE_SIZE;
template<class Q>
size_t const MemoryQueueTypedTest<Q>::MAX_ELEMENT_SIZE;
template<class Q>
size_t const MemoryQueueTypedTest<Q>::NUM_POSSIBLE_MAX_ELEMENT_SIZE_ALLOCATIONS;

// clang-format on

struct Pdu
{
    Pdu(uint8_t b, uint32_t i, ::estd::slice<uint8_t> pld) : bus(b), id(i), payload(pld) {}

    uint8_t bus;
    uint32_t id;
    ::estd::slice<uint8_t> payload;
};

template<class Queue>
::estd::slice<uint8_t>
allocate(typename Queue::Writer& s, uint8_t const bus, uint32_t const id, size_t const size)
{
    ::estd::slice<uint8_t> memory = s.allocate(sizeof(uint8_t) + sizeof(uint32_t) + size);
    if (memory.size() != 0)
    {
        ::estd::memory::take<uint8_t>(memory)             = bus;
        ::estd::memory::take<::estd::be_uint32_t>(memory) = id;
    }
    return memory;
}

template<class Queue>
Pdu poll(typename Queue::Reader& r)
{
    ::estd::slice<uint8_t> s = r.peek();
    if (s.size() != 0)
    {
        uint8_t const bus = ::estd::memory::take<uint8_t>(s);
        uint32_t const id = ::estd::memory::take<::estd::be_uint32_t>(s);
        return Pdu(bus, id, s);
    }
    return Pdu(0xFFU, 0xFFFFFFFFU, {});
}

using MemoryQueueTypes = ::testing::Types<
    ::io::MemoryQueue<20, 8>,
    ::io::MemoryQueue<40, 8>,
    ::io::MemoryQueue<128, 7>,
    ::io::MemoryQueue<17, 3>,
    ::io::MemoryQueue<17, 2>,
    ::io::MemoryQueue<17, 1>,
    ::io::MemoryQueue<17, 3, uint32_t>,
    ::io::MemoryQueue<17, 2, uint32_t>,
    ::io::MemoryQueue<17, 1, uint32_t>,
    ::io::MemoryQueue<17, 3, uint8_t>,
    ::io::MemoryQueue<17, 2, uint8_t>,
    ::io::MemoryQueue<17, 1, uint8_t>,
    ::io::MemoryQueue<3, 1>>;

TYPED_TEST_SUITE(MemoryQueueTypedTest, MemoryQueueTypes);

/**
 * \refs:    SMD_io_MemoryQueue
 * \desc
 * Checks that the constant capacity has the correct value
 */
TYPED_TEST(MemoryQueueTypedTest, capacity)
{
    ASSERT_TRUE(this->QUEUE_SIZE == (TypeParam::capacity()));
}

/**
 * \refs:    SMD_io_MemoryQueue
 * \desc
 * Tests that initially all bytes are available.
 */
TYPED_TEST(MemoryQueueTypedTest, capacity_initially_available)
{
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.available());
    EXPECT_EQ(this->QUEUE_SIZE, this->_r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * Tests that initially the minimum available bytes is capacity.
 */
TYPED_TEST(MemoryQueueTypedTest, min_available_initial_capacity)
{
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.minAvailable());
}

/**
 * \refs:    SMD_io_MemoryQueue
 * \desc
 * Test for default constructor.
 */
TYPED_TEST(MemoryQueueTypedTest, can_be_default_constructed)
{
    TypeParam q;
    SUCCEED();
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * If the number of bytes trying to allocate exceeds MAX_SIZE_ELEMENT, an empty slice is returned.
 */
TYPED_TEST(MemoryQueueTypedTest, allocate_returns_empty_slice_if_requested_size_is_too_big)
{
    ::estd::slice<uint8_t> s = this->_w.allocate(TypeParam::maxElementSize() + 1);
    EXPECT_EQ(0, s.size());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * If available() returns 0, an empty slice is returned.
 */
TYPED_TEST(MemoryQueueTypedTest, allocate_returns_empty_slice_if_available_is_zero)
{
    EXPECT_NE(0, this->_w.available());
    EXPECT_NE(0, this->_r.available());
    for (size_t a = 0; a < this->NUM_POSSIBLE_MAX_ELEMENT_SIZE_ALLOCATIONS; ++a)
    {
        ::estd::slice<uint8_t> s = this->_w.allocate(TypeParam::maxElementSize());
        EXPECT_EQ(TypeParam::maxElementSize(), s.size());
        this->_w.commit();
    }
    size_t const available = this->_w.available();
    if (available > 0)
    {
        size_t const allocate    = available - sizeof(typename TypeParam::size_type);
        ::estd::slice<uint8_t> s = this->_w.allocate(allocate);
        this->_w.commit();
        EXPECT_EQ(allocate, s.size());
    }
    EXPECT_EQ(0, this->_w.available());
    EXPECT_EQ(0, this->_r.available());
    ::estd::slice<uint8_t> s = this->_w.allocate(TypeParam::maxElementSize());
    EXPECT_EQ(0, s.size());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * This test assures that calling commit() on a writer without previous allocation leaves the
 * queue untouched.
 */
TYPED_TEST(MemoryQueueTypedTest, committing_without_allocation_has_no_effect)
{
    this->_w.commit();
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.available());
    EXPECT_EQ(this->QUEUE_SIZE, this->_r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * This test makes sure, that empty slices can't be allocated. Such an allocation would eat up
 * sizeof(SIZE_TYPE) bytes but the result of allocate would be an empty slice and this couldn't
 * be distinguished from an error case, where also an empty slice is returned.
 */
TYPED_TEST(MemoryQueueTypedTest, allocating_empty_slices_is_not_possible)
{
    size_t const writeAvailable = this->_w.available();
    EXPECT_NE(0, writeAvailable);
    size_t const readAvailable = this->_r.available();
    EXPECT_NE(0, readAvailable);

    auto const b = this->_w.allocate(0);
    this->_w.commit();
    EXPECT_EQ(0, b.size());
    EXPECT_EQ(nullptr, b.data());
    EXPECT_EQ(writeAvailable, this->_w.available());
    EXPECT_EQ(readAvailable, this->_r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Reader
 * \desc
 * Calling peek on an empty MemoryQueue must return an empty slice.
 */
TYPED_TEST(MemoryQueueTypedTest, peek_on_empty_reader_returns_empty_slice)
{
    auto const b = this->_r.peek();
    EXPECT_EQ(0, b.size());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * If less than MAX_SIZE_ELEMENT bytes remain for allocation, available must return zero.
 */
TYPED_TEST(
    MemoryQueueTypedTest, available_returns_zero_if_less_then_max_element_size_bytes_are_available)
{
    if (this->NUM_POSSIBLE_MAX_ELEMENT_SIZE_ALLOCATIONS > 0)
    {
        for (size_t a = 0; a < this->NUM_POSSIBLE_MAX_ELEMENT_SIZE_ALLOCATIONS - 1; ++a)
        {
            ::estd::slice<uint8_t> const s = this->_w.allocate(TypeParam::maxElementSize());
            EXPECT_EQ(TypeParam::maxElementSize(), s.size());
            this->_w.commit();
        }
    }
    EXPECT_GT(this->_w.available(), TypeParam::maxElementSize());
    EXPECT_GT(this->_r.available(), TypeParam::maxElementSize());
    (void)this->_w.allocate(TypeParam::maxElementSize());
    this->_w.commit();
    EXPECT_EQ(0U, this->_w.available());
    EXPECT_EQ(0U, this->_r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * Before committing the first data, the Reader must be empty.
 */
TYPED_TEST(MemoryQueueTypedTest, reader_is_empty_before_first_commit)
{
    EXPECT_TRUE(this->_r.empty());
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.available());
    EXPECT_EQ(this->QUEUE_SIZE, this->_r.available());
    (void)this->_w.allocate(TypeParam::maxElementSize());
    EXPECT_TRUE(this->_r.empty());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * After committing the first data, the Reader must not be empty.
 */
TYPED_TEST(MemoryQueueTypedTest, receiver_is_not_empty_after_first_commit)
{
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.available());
    EXPECT_EQ(this->QUEUE_SIZE, this->_r.available());
    (void)this->_w.allocate(TypeParam::maxElementSize());
    this->_w.commit();
    EXPECT_FALSE(this->_r.empty());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * Just calling peek on the Receiver must not free any bytes in a MemoryQueue which has no bytes
 * available.
 */
TEST_F(MemoryQueueTest, reader_peek_does_not_make_bytes_available)
{
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());

    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());
    (void)_w.allocate(4U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());

    auto const b = _r.peek();
    EXPECT_EQ(Q::maxElementSize(), b.size());
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());
    EXPECT_TRUE(_w.full());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * After calling release, at least the size of the released slice plus sizeof(SIZE_TYPE) bytes
 * become available.
 */
TEST_F(MemoryQueueTest, release_makes_size_plus_sizeof_sizetype_bytes_available)
{
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());

    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());
    (void)_w.allocate(4U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());

    auto const b = _r.peek();
    EXPECT_EQ(Q::maxElementSize(), b.size());
    _r.release();
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * Calling clear() on a reader will release elements until the queue is empty.
 */
TEST_F(MemoryQueueTest, calling_clear_on_reader_empties_queue)
{
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());

    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());
    (void)_w.allocate(4U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());

    EXPECT_TRUE(_w.full());
    EXPECT_FALSE(_r.empty());
    _r.clear();
    EXPECT_FALSE(_w.full());
    EXPECT_TRUE(_r.empty());
    EXPECT_EQ(20U, _w.available());
    EXPECT_EQ(20U, _r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * Calling release() on an empty queue shall leave it empty.
 */
TYPED_TEST(MemoryQueueTypedTest, release_on_empty_queue_has_no_effect)
{
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.available());
    EXPECT_EQ(this->QUEUE_SIZE, this->_r.available());
    EXPECT_TRUE(this->_r.empty());
    this->_r.release();
    EXPECT_EQ(this->QUEUE_SIZE, this->_w.available());
    EXPECT_EQ(this->QUEUE_SIZE, this->_r.available());
    EXPECT_TRUE(this->_r.empty());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * If more or exactly MAX_SIZE_ELEMENT + sizeof(SIZE_TYPE) bytes remain for allocation,
 * available must not return zero.
 */
TEST_F(MemoryQueueTest, available_returns_non_zero_if_at_least_max_element_size_bytes_are_available)
{
    (void)_w.allocate(5U);
    _w.commit();
    EXPECT_EQ(13U, _w.available());
    EXPECT_EQ(13U, _r.available());
    // this will actually eat 3 bytes
    (void)_w.allocate(1U);
    _w.commit();
    EXPECT_NE(0U, _w.available());
    EXPECT_NE(0U, _r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * Calling allocate multiple times is possible, the last allocation wins.
 */
TEST_F(MemoryQueueTest, allocate_can_be_used_to_reallocate_different_size_before_calling_commit)
{
    auto b = _w.allocate(5);
    EXPECT_EQ(5, b.size());

    // nothing committed yet
    EXPECT_EQ(0, _r.peek().size());

    // reallocate with different size
    b = _w.allocate(3);
    EXPECT_EQ(3, b.size());
    _w.commit();
    EXPECT_EQ(3, _r.peek().size());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * Calling allocate multiple times is possible, the last allocation with zero will make a following
 * call to commit have no effect.
 */
TEST_F(MemoryQueueTest, reallocate_with_zero_will_cancel_allocation)
{
    size_t const writeAvailable = _w.available();
    size_t const readAvailable  = _r.available();

    auto b = _w.allocate(5);
    EXPECT_EQ(5, b.size());

    // nothing committed yet
    EXPECT_EQ(0, _r.peek().size());

    // reallocate with different size
    b = _w.allocate(0);
    EXPECT_EQ(0, b.size());
    _w.commit();
    EXPECT_EQ(_w.available(), writeAvailable);
    EXPECT_EQ(_r.available(), readAvailable);
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * Allocating some data to the queue causes minAvailable() to be lower than CAPACITY
 * The tested queue has a capacity of 20 bytes and a maximum allocation size of 8, size_type is
 * uint16_t.
 */
TEST_F(MemoryQueueTest, min_available_lower_than_capacity)
{
    // Allocate 8 bytes (+2 bytes for size) -> 10 bytes remain
    // [s s 1 1 1 1 1 1 1 1 # # # # # # # # # #]
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    (void)_w.allocate(Q::maxElementSize());
    EXPECT_EQ(10U, _w.minAvailable());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * Filling up the queue before emptying it makes minAvailable go to 0.
 * The tested queue has a capacity of 20 bytes and a maximum allocation size of 8, size_type is
 * uint16_t.
 */
TEST_F(MemoryQueueTest, min_available_if_full)
{
    // 1) Allocate 8 bytes (+2 bytes for size) -> 10 bytes remain
    // [s s 1 1 1 1 1 1 1 1 # # # # # # # # # #]
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    // 2) Allocate 4 bytes (+2) -> 4 bytes remain -> full
    // [s s 1 1 1 1 1 1 1 1 s s 2 2 2 2 # # # #]
    (void)_w.allocate(4U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());
    // This call fails and sets minAvailable()
    EXPECT_EQ(0U, _w.allocate(4U).size());
    EXPECT_EQ(0U, _w.minAvailable());

    // 3) Release 8 bytes (+2) -> 10 bytes available because the remaining 4 bytes and freed 10
    // are not contiguous
    // [# # # # # # # # # # s s 2 2 2 2 # # # #]
    auto b = _r.peek();
    EXPECT_EQ(Q::maxElementSize(), b.size());
    _r.release();
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    // 4) Release second chunk 4 (+2) -> queue is empty
    // [# # # # # # # # # # # # # # # # # # # #]
    b = _r.peek();
    EXPECT_EQ(4U, b.size());
    _r.release();
    EXPECT_EQ(20U, _w.available());
    EXPECT_EQ(20U, _r.available());
    EXPECT_TRUE(_r.empty());

    EXPECT_EQ(0U, _w.minAvailable());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer
 * \desc
 * minAvailable goes back to capacity if it is reset.
 * The tested queue has a capacity of 20 bytes and a maximum allocation size of 8, size_type is
 * uint16_t.
 */
TEST_F(MemoryQueueTest, min_available_reset)
{
    // 1) Allocate 8 bytes (+2 bytes for size) -> 10 bytes remain
    // [s s 1 1 1 1 1 1 1 1 # # # # # # # # # #]
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    // 2) Allocate 4 bytes (+2) -> 4 bytes remain -> full
    // [s s 1 1 1 1 1 1 1 1 s s 2 2 2 2 # # # #]
    (void)_w.allocate(4U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());
    // This call fails and sets minAvailable()
    EXPECT_EQ(0U, _w.allocate(4U).size());
    EXPECT_EQ(0U, _w.minAvailable());

    // 3) Release 8 bytes (+2) -> 10 bytes available because the remaining 4 bytes and freed 10
    // are not contiguous
    // [# # # # # # # # # # s s 2 2 2 2 # # # #]
    auto b = _r.peek();
    EXPECT_EQ(Q::maxElementSize(), b.size());
    _r.release();
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    // 4) Release second chunk 4 (+2) -> queue is empty
    // [# # # # # # # # # # # # # # # # # # # #]
    b = _r.peek();
    EXPECT_EQ(4U, b.size());
    _r.release();
    EXPECT_EQ(20U, _w.available());
    EXPECT_EQ(20U, _r.available());
    EXPECT_TRUE(_r.empty());

    EXPECT_EQ(0U, _w.minAvailable());

    // Since the queue is empty, reset to CAPACITY
    _w.resetMinAvailable();

    EXPECT_EQ(_q.capacity(), _w.minAvailable());
}

/**
 * \refs:    SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * Different allocation scenarios to make to available always returns expected result.
 * The tested queue has a capacity of 20 bytes and a maximum allocation size of 8, size_type is
 * uint16_t.
 */
TEST_F(MemoryQueueTest, available_use_cases)
{
    // 1) Allocate 8 bytes (+2 bytes for size) -> 10 bytes remain
    // [s s 1 1 1 1 1 1 1 1 # # # # # # # # # #]
    (void)_w.allocate(Q::maxElementSize());
    _w.commit();
    EXPECT_FALSE(_r.empty());
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    // 2) Allocate 4 bytes (+2) -> 4 bytes remain -> full
    // [s s 1 1 1 1 1 1 1 1 s s 2 2 2 2 # # # #]
    (void)_w.allocate(4U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());

    // 3) Release 8 bytes (+2) -> 10 bytes available because the remaining 4 bytes and freed 10
    // are not contiguous
    // [# # # # # # # # # # s s 2 2 2 2 # # # #]
    auto b = _r.peek();
    EXPECT_EQ(Q::maxElementSize(), b.size());
    _r.release();
    EXPECT_EQ(10U, _w.available());
    EXPECT_EQ(10U, _r.available());

    // 4) Release second chunk 4 (+2) -> queue is empty
    // [# # # # # # # # # # # # # # # # # # # #]
    b = _r.peek();
    EXPECT_EQ(4U, b.size());
    _r.release();
    EXPECT_EQ(20U, _w.available());
    EXPECT_EQ(20U, _r.available());
    EXPECT_TRUE(_r.empty());

    // 5) Allocate 6 (+2) bytes -> 12 bytes available
    // [s s 1 1 1 1 1 1 # # # # # # # # # # # #]
    (void)_w.allocate(6U);
    _w.commit();
    EXPECT_EQ(12U, _w.available());
    EXPECT_EQ(12U, _r.available());

    // 6) Allocate 6 (+2) bytes -> full (only 4 bytes remain)
    // [s s 1 1 1 1 1 1 s s 2 2 2 2 2 2 # # # #]
    (void)_w.allocate(6U);
    _w.commit();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());

    // 7) Release 6 (+2) bytes -> still full as not enough contiguous space for 8 (+2) bytes
    // available, only 4 at the end and 8 at the beginning
    // [# # # # # # # # s s 2 2 2 2 2 2 # # # #]
    b = _r.peek();
    EXPECT_EQ(6U, b.size());
    _r.release();
    EXPECT_EQ(0U, _w.available());
    EXPECT_EQ(0U, _r.available());

    // 8) Release second chunk of 6 (+2) bytes -> empty
    // [# # # # # # # # # # # # # # # # # # # #]
    b = _r.peek();
    EXPECT_EQ(6U, b.size());
    _r.release();
    EXPECT_EQ(20U, _w.available());
    EXPECT_EQ(20U, _r.available());
}

/**
 * \refs:    SMD_io_MemoryQueue, SMD_io_MemoryQueue::Writer, SMD_io_MemoryQueue::Reader
 * \desc
 * This test uses different sizes of entries and writes them repeatedly to a queue.
 */
TEST_F(MemoryQueueTest, stress_test_with_different_sizes)
{
    size_t const MAX_ELEMENT_SIZE = 4;
    ::io::MemoryQueue<17, MAX_ELEMENT_SIZE, uint8_t> q;
    ::io::MemoryQueue<17, MAX_ELEMENT_SIZE, uint8_t>::Writer w(q);
    ::io::MemoryQueue<17, MAX_ELEMENT_SIZE, uint8_t>::Reader r(q);

    for (size_t s = 1U; s <= MAX_ELEMENT_SIZE; ++s)
    {
        for (uint8_t i = 0U; i < 255; ++i)
        {
            ASSERT_FALSE(w.full()) << "at: " << i;
            {
                auto const b = w.allocate(s);
                ::estd::memory::set(b, i);
                w.commit();
            }
            ASSERT_FALSE(r.empty());
            {
                auto const b = r.peek();
                EXPECT_EQ(s, b.size());
                EXPECT_THAT(b, Each(Eq(i)));
                r.release();
            }
        }
    }
}

/**
 * \refs:    SMD_io_MemoryQueueWriter, SMD_io_MemoryQueueReader, SMD_io_IReader, SMD_io_IWriter
 * \desc
 * Illustrates some use cases of IWriter/IReader
 */
TEST_F(MemoryQueueTest, WriterReader_use_cases)
{
    ::io::MemoryQueueWriter<Q> mqw(_q);
    ::io::IWriter& w = mqw;
    ::io::MemoryQueueReader<Q> mqr(_q);
    ::io::IReader& r = mqr;

    // 0) Check maximum element sizes, available buffer size
    EXPECT_EQ(MAX_ELEMENT_SIZE, w.maxSize());
    EXPECT_EQ(MAX_ELEMENT_SIZE, r.maxSize());
    EXPECT_EQ(this->QUEUE_SIZE, mqw.available());
    EXPECT_EQ(this->QUEUE_SIZE, mqr.available());

    // 1) Allocate 8 bytes (+2 bytes for size) -> 10 bytes remain available
    (void)w.allocate(Q::maxElementSize());
    w.commit();
    EXPECT_EQ(this->QUEUE_SIZE - (Q::maxElementSize() + sizeof(Q::size_type)), mqw.available());
    EXPECT_EQ(this->QUEUE_SIZE - (Q::maxElementSize() + sizeof(Q::size_type)), mqr.available());

    // 2) Allocate 4 (+2) bytes
    auto const data = w.allocate(4U);
    ASSERT_EQ(4U, data.size());
    w.commit();

    // 3) Read and release the first chunk of allocated bytes (as ConstBytes)
    auto b = r.peek();
    EXPECT_EQ(Q::maxElementSize(), b.size());
    r.release();

    // 4) Read and release second chunk -> queue empty
    b = r.peek();
    EXPECT_EQ(4U, b.size());
    r.release();
    EXPECT_EQ(0U, r.peek().size());

    // 5) Allocate and commit two times 6 (+2) bytes
    (void)w.allocate(6U);
    w.commit();
    (void)w.allocate(6U);
    w.commit();

    // 6) Read and release the two chunks -> queue empty
    b = r.peek();
    EXPECT_EQ(6U, b.size());
    r.release();
    b = r.peek();
    EXPECT_EQ(6U, b.size());
    r.release();
    EXPECT_EQ(0U, r.peek().size());
}

/**
 * \refs:    SMD_io_IReader
 * \desc
 * Test for ReleaseGuard protecting an instance of IReader.
 */
TEST_F(MemoryQueueTest, ReleaseGuard_releases_data_when_going_out_of_scope)
{
    ::io::MemoryQueueWriter<Q> w(_q);
    ::io::MemoryQueueReader<Q> r(_q);

    // 1) guard on empty reader has no effect
    {
        ::io::ReleaseGuard g(r);
        (void)g;
    }

    // 2) allocate 1 (+2) byte(s) and 4 (+2) bytes
    auto b = w.allocate(1);
    EXPECT_EQ(1, b.size());
    w.commit();
    b = w.allocate(4);
    EXPECT_EQ(4, b.size());
    w.commit();

    // 3) Read first chunk and let guard release the data
    {
        ::io::ReleaseGuard g(r);
        auto const cb = g.peek();
        EXPECT_EQ(1, cb.size());
    }
    // 4) Let guard release the data without accessing it
    {
        ::io::ReleaseGuard g(r);
        (void)g;
    }
    EXPECT_EQ(0, r.peek().size());
}

/**
 * \example
 * This example shows how two custom functions (allocate/poll) can be used to create an abstraction
 * for data types.
 *
 * Here, the contract is that elements transmitted through the queue consist of:
 * - uint8_t identifying the bus
 * - uint32_t bus specific id
 * - bytes of payload
 *
 * Please note that there is no need to store the length of the payload explicitly as it can be
 * implicitly computed from the size of the slice returned by peek().
 */
TEST_F(MemoryQueueTest, PduType)
{
    using MQ = ::io::MemoryQueue<1024, 64>;
    MQ q;
    MQ::Writer w(q);
    MQ::Reader r(q);

    // 1) allocate() allocates a slice of payload data and writes bus and PDU Id to the queue
    ::estd::slice<uint8_t> pld = allocate<MQ>(w, 1, 0x1234U, 10);
    pld[0]                     = 0x11U;
    pld[1]                     = 0x22U;
    pld[2]                     = 0x33U;
    w.commit();

    // 2) poll() retrieves an elements of type Pdu
    Pdu rxPdu = poll<MQ>(r);
    EXPECT_EQ(1, rxPdu.bus);
    EXPECT_EQ(0x1234, rxPdu.id);
    ASSERT_EQ(10, rxPdu.payload.size());
    EXPECT_EQ(0x11, rxPdu.payload[0]);
    EXPECT_EQ(0x22, rxPdu.payload[1]);
    EXPECT_EQ(0x33, rxPdu.payload[2]);
    r.release();
}

/**
 * \example
 * Example from estd_io.md
 */
TEST_F(MemoryQueueTest, Example1)
{
    // [example1]
    using namespace ::io;

    using Queue = MemoryQueue<10, 2, uint16_t>;
    Queue q;
    MemoryQueueWriter<Queue> w(q);
    MemoryQueueReader<Queue> r(q);

    // First allocation will use three bytes
    auto b = w.allocate(1);
    ASSERT_EQ(1, b.size());
    b[0] = 0xAU;
    w.commit();
    // [00 01 0A xx xx xx xx xx xx xx]

    // Second allocation will use three bytes
    b = w.allocate(1);
    ASSERT_EQ(1, b.size());
    b[0] = 0xBU;
    w.commit();
    // [00 01 0A 00 01 0B xx xx xx xx]

    // Third allocation will use three bytes
    b = w.allocate(1);
    ASSERT_EQ(1, b.size());
    b[0] = 0xCU;
    w.commit();
    // [00 01 0A 00 01 0B 00 01 0C xx]

    // now only one byte is available, no more allocations possible
    b = w.allocate(1);
    ASSERT_EQ(0, b.size());
    b = r.peek();
    r.release();
    // [xx xx xx 00 01 0B 00 01 0C xx]

    // now a total of 4 bytes is available but still no allocation is possible
    // as the 4 bytes are not contiguous
    b = w.allocate(1);
    ASSERT_EQ(0, b.size());
    // [example1]
}

} // namespace
