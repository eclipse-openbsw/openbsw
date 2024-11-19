// Copyright 2024 Accenture.

#include "util/spsc/Queue.h"

#include "util/spsc/ReadWrite.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

// The tested types can be constructed from an integer. This makes it easier to compare them
// especially in typed tests.

struct X
{
    X() : x(0) {}

    X(int i) : x(i) {}

    int x;
};

bool operator==(X const& lhs, X const& rhs) { return lhs.x == rhs.x; }

struct SpSc : public ::testing::Test
{
    using IntQueue    = util::spsc::Queue<uint16_t, 6>;
    using StructQueue = util::spsc::Queue<X, 6>;

    IntQueue queue;
    IntQueue::Sender sender;
    IntQueue::Receiver receiver;

    SpSc() : sender(queue), receiver(queue) {}
};

template<typename T>
struct SpScQueueWriterReader : public ::testing::Test
{
    static size_t const QUEUE_SIZE = 6U;
    using Queue                    = util::spsc::Queue<T, QUEUE_SIZE>;

    Queue queue;
    util::spsc::QueueWriter<Queue> writer;
    util::spsc::QueueReader<Queue> reader;

    SpScQueueWriterReader() : writer(queue), reader(queue) {}
};

template<typename T>
size_t const SpScQueueWriterReader<T>::QUEUE_SIZE;

template<typename T>
struct SpScWriterReader : public ::testing::Test
{
    static size_t const QUEUE_SIZE = 6U;
    using Queue                    = util::spsc::Queue<T, QUEUE_SIZE>;
    Queue queue;

    util::spsc::QueueWriter<Queue> qWriter;
    util::spsc::QueueReader<Queue> qReader;

    util::spsc::Writer<T> writer;
    util::spsc::Reader<T> reader;

    SpScWriterReader() : qWriter(queue), qReader(queue), writer(qWriter), reader(qReader) {}

    size_t insertNumberedElementsUntilFull()
    {
        size_t insertedElements = 0;
        while (!this->writer.full())
        {
            T const element(insertedElements);
            this->writer.write(element);
            ++insertedElements;
        }
        return insertedElements;
    }

    size_t readNumberedElementsUntilEmpty()
    {
        size_t readElements = 0;
        while (!this->reader.empty())
        {
            T const readElement(readElements);
            T const e = this->reader.read();
            EXPECT_EQ(readElement, e);
            ++readElements;
        }
        return readElements;
    }
};

template<typename T>
size_t const SpScWriterReader<T>::QUEUE_SIZE;

using MyTypes = ::testing::Types<uint16_t, X>;

TYPED_TEST_SUITE(SpScWriterReader, MyTypes);

/**
 * \desc
 * Checks that capacity() of Queue returns the correct value.
 */
TYPED_TEST(SpScWriterReader, Capacity)
{
    EXPECT_EQ(SpScWriterReader<TypeParam>::QUEUE_SIZE, this->queue.capacity());
}

/**
 * \desc
 * This test verifies that the Writer is not full after being created.
 */
TYPED_TEST(SpScWriterReader, WriterNotFullAfterCreating) { EXPECT_FALSE(this->writer.full()); }

/**
 * \desc
 * This test verifies that the Reader is empty after being created.
 */
TYPED_TEST(SpScWriterReader, ReaderEmptyAfterCreating) { EXPECT_TRUE(this->reader.empty()); }

/**
 * \desc
 * Verifies that the Reader sees one element after it has been written and checks that its
 * content is equal to the inserted.
 */
TYPED_TEST(SpScWriterReader, WriteAndReadOneElement)
{
    TypeParam const element(10);
    this->writer.write(element);
    EXPECT_FALSE(this->reader.empty());
    EXPECT_EQ(1U, this->reader.size());
    TypeParam readElement = this->reader.read();
    EXPECT_EQ(element, readElement);
    EXPECT_TRUE(this->reader.empty());
}

/**
 * \desc
 * Inserts elements until the Writer returns full and checks that capacity elements were added.
 */
TYPED_TEST(SpScWriterReader, WriteUntilFullAndCheckThatCapacityElementsHaveBeenInserted)
{
    // Fill queue with different elements
    auto const insertedElements = this->insertNumberedElementsUntilFull();
    EXPECT_EQ(this->queue.capacity(), insertedElements);
}

/**
 * \desc
 * Inserts elements until the Writer returns full and checks that the same number of elements
 * can be read and their values match the inserted ones.
 */
TYPED_TEST(SpScWriterReader, WriteUntilFullAndReadElementsUntilEmpty)
{
    auto const insertedElements = this->insertNumberedElementsUntilFull();
    auto const readElements     = this->readNumberedElementsUntilEmpty();

    EXPECT_EQ(insertedElements, readElements);
}

/**
 * \desc
 * Verifies that the pointer and reference operators point to the same memory.
 * Also the memory() function returns the same address.
 */
TYPED_TEST(SpScWriterReader, WriterWriteAccessOperatorsAccessSameElement)
{
    typename util::spsc::Writer<TypeParam>::Write w = this->writer.create();
    TypeParam& element                              = *w;
    EXPECT_EQ(&element, w.operator->());
    EXPECT_EQ(&element, w.memory());
}

/**
 * \desc
 * Verifies that the pointer and reference operators point to the same memory.
 */
TYPED_TEST(SpScWriterReader, ReaderReadAccessOperatorsAccessSameElement)
{
    this->writer.write(TypeParam(17));
    typename util::spsc::Reader<TypeParam>::Read r = this->reader.create();
    TypeParam const& element                       = *r;
    EXPECT_EQ(&element, r.operator->());
}

/**
 * \desc
 * Verifies that using a Writer::Write object to fill the queue and a Reader::Read to
 * empty it works the same way as using Writer and Reader API directly.
 */
TYPED_TEST(SpScWriterReader, WriterWriteUntilFullAndUseReaderReadUntilEmpty)
{
    // Fill queue with different elements
    size_t insertedElements = 0;
    while (!this->writer.full())
    {
        TypeParam const element(insertedElements);
        typename util::spsc::Writer<TypeParam>::Write w = this->writer.create();
        *w                                              = element;
        ++insertedElements;
    }
    EXPECT_EQ(this->queue.capacity(), insertedElements);
    // read them back
    size_t readElements = 0;
    while (!this->reader.empty())
    {
        TypeParam const readElement(readElements);
        typename util::spsc::Reader<TypeParam>::Read r = this->reader.create();
        EXPECT_EQ(readElement, *r);
        ++readElements;
    }
    EXPECT_EQ(insertedElements, readElements);
}

/**
 * \desc
 * Verifies that using emplace() to insert elements works.
 */
TYPED_TEST(SpScWriterReader, EmplaceUntilFullAndReadElementsUntilEmpty)
{
    // Fill queue with different elements
    int insertedElements = 0;
    while (!this->writer.full())
    {
        this->writer.emplace().construct(insertedElements);
        ++insertedElements;
    }
    int readElements = this->readNumberedElementsUntilEmpty();

    EXPECT_EQ(insertedElements, readElements);
}

template<class T>
struct Producer
{
    T _i;

    Producer(int i) : _i(i) {}

    void operator()(T& value) const { value = _i; }
};

/**
 * \desc
 * Verifies that using a Producer to insert elements works.
 */
TYPED_TEST(SpScWriterReader, ProduceUntilFullAndReadElementsUntilEmpty)
{
    // Fill queue with different elements
    int insertedElements = 0;
    while (!this->writer.full())
    {
        Producer<TypeParam> p(insertedElements);
        this->writer.write(p);
        ++insertedElements;
    }
    int readElements = this->readNumberedElementsUntilEmpty();

    EXPECT_EQ(insertedElements, readElements);
}

template<class T>
struct Consumer
{
    T _i;

    Consumer(int i) : _i(i) {}

    void operator()(T const& value) const { EXPECT_EQ(value, _i); }
};

/**
 * \desc
 * Verifies that using a Consumer to read the elements works.
 */
TYPED_TEST(SpScWriterReader, WriteUntilFullAndUseReadWithConsumerUntilEmpty)
{
    int insertedElements = this->insertNumberedElementsUntilFull();
    // read them back
    int readElements     = 0;
    while (!this->reader.empty())
    {
        Consumer<TypeParam> c(readElements);
        this->reader.read(c);
        ++readElements;
    }

    EXPECT_EQ(insertedElements, readElements);
}

TYPED_TEST(SpScWriterReader, WriteUntilFullAndUseReaderUntilEmpty)
{
    int insertedElements = this->insertNumberedElementsUntilFull();

    ::util::spsc::IReader<TypeParam>& r = this->reader.reader();
    // read them back
    int readElements                    = 0;
    while (!r.empty())
    {
        TypeParam const expected(readElements);
        TypeParam const& v = r.peek();
        EXPECT_EQ(expected, v);
        r.advance();
        ++readElements;
    }

    EXPECT_EQ(insertedElements, readElements);
}

TYPED_TEST_SUITE(SpScQueueWriterReader, MyTypes);

/**
 * \desc
 * This test assures that a QueueWriter is not full after the queue has been created.
 */
TYPED_TEST(SpScQueueWriterReader, WriterNotFullAfterCreating) { EXPECT_FALSE(this->writer.full()); }

/**
 * \desc
 * This test assures that a QueueReader is empty after creation.
 */
TYPED_TEST(SpScQueueWriterReader, ReaderEmptyAfterCreating) { EXPECT_TRUE(this->reader.empty()); }

/**
 * \desc
 * This test assures that capacity() elements are inserted when a previously empty queue becomes
 * full using the write() function to insert elements.
 */
TYPED_TEST(SpScQueueWriterReader, WriterWriteUntilFull)
{
    size_t insertedElements = 0;
    while (!this->writer.full())
    {
        TypeParam const element(insertedElements);
        this->writer.write(element);
        ++insertedElements;
    }
    EXPECT_EQ(this->queue.capacity(), insertedElements);
}

/**
 * \desc
 * This test assures that capacity() elements are inserted when a previously empty queue becomes
 * full using the next() and commit() function to insert elements.
 */
TYPED_TEST(SpScQueueWriterReader, WriterNextCommitUntilFull)
{
    size_t insertedElements = 0;
    while (!this->writer.full())
    {
        TypeParam const element(insertedElements);
        this->writer.next() = element;
        this->writer.commit();
        ++insertedElements;
    }
    EXPECT_EQ(this->queue.capacity(), insertedElements);
}

/**
 * \desc
 * This test assures that capacity() elements are inserted when a previously empty queue becomes
 * full using IWriter<T>::write() function to insert elements.
 */
TYPED_TEST(SpScQueueWriterReader, IWriterWriteUntilFull)
{
    size_t insertedElements = 0;
    while (!this->writer.full())
    {
        TypeParam const element(insertedElements);
        util::spsc::IWriter<TypeParam>& w = this->writer;
        w.write(element);
        ++insertedElements;
    }
    EXPECT_EQ(this->queue.capacity(), insertedElements);
}

/**
 * \desc
 * This test verifies that calling size() on a QueueReader always returns the number of elements
 * that have been written by a QueueWriter.
 */
TYPED_TEST(SpScQueueWriterReader, ReaderSizeIsNumberOfInsertedElements)
{
    size_t insertedElements = 0;
    while (!this->writer.full())
    {
        TypeParam const element(insertedElements);
        this->writer.write(element);
        ++insertedElements;
        EXPECT_EQ(insertedElements, this->reader.size());
    }
}

TEST_F(SpSc, is_initially_empty) { ASSERT_TRUE(receiver.empty()); }

TEST_F(SpSc, can_be_pushed_and_poped_a_single_element)
{
    sender.write(4);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(4, receiver.read());
    ASSERT_TRUE(receiver.empty());

    sender.write(5);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(5, receiver.read());
    ASSERT_TRUE(receiver.empty());
}

TEST_F(SpSc, can_be_pushed_and_poped_two_elements)
{
    sender.write(4);
    ASSERT_FALSE(receiver.empty());
    sender.write(5);
    ASSERT_FALSE(receiver.empty());

    ASSERT_EQ(4, receiver.read());
    ASSERT_FALSE(receiver.empty());

    ASSERT_EQ(5, receiver.read());
    ASSERT_TRUE(receiver.empty());
}

TEST_F(SpSc, the_Sender_can_be_created_multiple_times)
{
    // Note that while it is possible to create multiple instances of Sender,
    // they all have to be used from the same thread.

    sender.write(4);
    ASSERT_FALSE(receiver.empty());

    IntQueue::Sender sender2(queue);
    sender2.write(5);
    ASSERT_FALSE(receiver.empty());

    ASSERT_EQ(4, receiver.read());
    ASSERT_FALSE(receiver.empty());

    ASSERT_EQ(5, receiver.read());
    ASSERT_TRUE(receiver.empty());
}

TEST_F(SpSc, the_Reveiver_can_be_created_multiple_times)
{
    // Note that while it is possible to create multiple instances of Receiver,
    // they all have to be used from the same thread.

    sender.write(4);
    ASSERT_FALSE(receiver.empty());
    sender.write(5);
    ASSERT_FALSE(receiver.empty());

    ASSERT_EQ(4, receiver.read());
    ASSERT_FALSE(receiver.empty());

    IntQueue::Receiver receiver2(queue);
    ASSERT_EQ(5, receiver2.read());
    ASSERT_TRUE(receiver.empty());
    ASSERT_TRUE(receiver2.empty());
}

TEST_F(SpSc, the_Reveiver_can_clear_the_queue)
{
    sender.write(4);
    ASSERT_FALSE(receiver.empty());
    sender.write(5);
    ASSERT_FALSE(receiver.empty());

    receiver.clear();
    ASSERT_TRUE(receiver.empty());
}

TEST_F(SpSc, two_step_write_avoids_copy)
{
    StructQueue queue;
    StructQueue::Sender sender(queue);
    StructQueue::Receiver receiver(queue);

    X& x = sender.next();
    x.x  = 2;
    EXPECT_TRUE(receiver.empty());
    sender.write_next();
    EXPECT_FALSE(receiver.empty());

    X read = receiver.read();
    EXPECT_TRUE(receiver.empty());
    ASSERT_EQ(2, read.x);
}

TEST_F(SpSc, scoped_write_avoids_copy)
{
    StructQueue queue;
    StructQueue::Sender sender(queue);
    StructQueue::Receiver receiver(queue);

    {
        StructQueue::Sender::Write write(sender);
        write->x = 2;
        EXPECT_TRUE(receiver.empty());
    }
    EXPECT_FALSE(receiver.empty());

    X read = receiver.read();
    EXPECT_TRUE(receiver.empty());
    ASSERT_EQ(2, read.x);
}

TEST_F(SpSc, two_step_read_avoids_copy)
{
    StructQueue queue;
    StructQueue::Sender sender(queue);
    StructQueue::Receiver receiver(queue);

    {
        X x;
        x.x = 4;
        sender.write(x);
    }

    X const& x = receiver.peek();
    EXPECT_EQ(4, x.x);
    EXPECT_FALSE(receiver.empty());

    receiver.advance();
    EXPECT_TRUE(receiver.empty());
}

TEST_F(SpSc, scoped_read_avoids_copy)
{
    StructQueue queue;
    StructQueue::Sender sender(queue);
    StructQueue::Receiver receiver(queue);

    X x;
    x.x = 4;
    sender.write(x);

    {
        StructQueue::Receiver::Read read(receiver);
        EXPECT_EQ(4, read->x);
        EXPECT_EQ(4, (*read).x);
        EXPECT_FALSE(receiver.empty());
    }
    EXPECT_TRUE(receiver.empty());
}

TEST_F(SpSc, can_be_full)
{
    sender.write(1);
    ASSERT_FALSE(sender.full());
    sender.write(2);
    ASSERT_FALSE(sender.full());
    sender.write(3);
    ASSERT_FALSE(sender.full());
    sender.write(4);
    ASSERT_FALSE(sender.full());
    sender.write(5);
    ASSERT_FALSE(sender.full());
    sender.write(6);
    ASSERT_TRUE(sender.full());

    receiver.read();
    ASSERT_FALSE(sender.full());
}

TEST_F(SpSc, overflow)
{
    // This test, and in particular the seemingly arbitrary constant "10" used below only
    // makes sense when knowing about the implementation detail of this queue that
    // internally the indices are stored in the range [0; 2*N]
    // 10 is chosen, because 10 < 2*N < 10+N
    for (size_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(0U, receiver.size()) << i;
        EXPECT_EQ(0U, sender.size()) << i;
        sender.write(uint16_t(i));
        EXPECT_EQ(1U, receiver.size()) << i;
        EXPECT_EQ(1U, sender.size()) << i;
        ASSERT_EQ(uint16_t(i), receiver.read());
        EXPECT_EQ(0U, receiver.size()) << i;
        EXPECT_EQ(0U, sender.size()) << i;
    }
    ASSERT_FALSE(sender.full());
    ASSERT_TRUE(receiver.empty());
    EXPECT_EQ(0U, receiver.size());
    EXPECT_EQ(0U, sender.size());

    sender.write(252);
    ASSERT_FALSE(receiver.empty());
    ASSERT_FALSE(sender.full());
    EXPECT_EQ(1U, receiver.size());
    EXPECT_EQ(1U, sender.size());

    sender.write(253);
    ASSERT_FALSE(receiver.empty());
    ASSERT_FALSE(sender.full());
    EXPECT_EQ(2U, receiver.size());
    EXPECT_EQ(2U, sender.size());

    sender.write(254);
    ASSERT_FALSE(receiver.empty());
    ASSERT_FALSE(sender.full());
    EXPECT_EQ(3U, receiver.size());
    EXPECT_EQ(3U, sender.size());

    sender.write(255);
    ASSERT_FALSE(receiver.empty());
    ASSERT_FALSE(sender.full());
    EXPECT_EQ(4U, receiver.size());
    EXPECT_EQ(4U, sender.size());

    sender.write(256);
    ASSERT_FALSE(receiver.empty());
    ASSERT_FALSE(sender.full());
    EXPECT_EQ(5U, receiver.size());
    EXPECT_EQ(5U, sender.size());

    sender.write(257);
    ASSERT_TRUE(sender.full());
    ASSERT_FALSE(receiver.empty());
    EXPECT_EQ(6U, receiver.size());
    EXPECT_EQ(6U, sender.size());

    ASSERT_EQ(252, receiver.read());
    EXPECT_EQ(5U, receiver.size());
    EXPECT_EQ(5U, sender.size());
    ASSERT_EQ(253, receiver.read());
    EXPECT_EQ(4U, receiver.size());
    EXPECT_EQ(4U, sender.size());
    ASSERT_EQ(254, receiver.read());
    EXPECT_EQ(3U, receiver.size());
    EXPECT_EQ(3U, sender.size());
    ASSERT_EQ(255, receiver.read());
    EXPECT_EQ(2U, receiver.size());
    EXPECT_EQ(2U, sender.size());
    ASSERT_EQ(256, receiver.read());
    EXPECT_EQ(1U, receiver.size());
    EXPECT_EQ(1U, sender.size());
    ASSERT_EQ(257, receiver.read());
    EXPECT_EQ(0U, receiver.size());
    EXPECT_EQ(0U, sender.size());

    ASSERT_FALSE(sender.full());
    ASSERT_TRUE(receiver.empty());
}

class OnConsumed
{
public:
    MOCK_CONST_METHOD1(called, void(uint16_t));

    void operator()(uint16_t& x) const { called(x); }
};

TEST_F(SpSc, consumed_notification)
{
    OnConsumed onConsumed;
    using IntQueueConsumed = util::spsc::Queue<uint16_t, 6, true>;

    IntQueueConsumed queue;
    IntQueueConsumed::Sender sender(queue);
    IntQueueConsumed::Receiver receiver(queue);

    sender.write(4);
    sender.write(2);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(4, receiver.read());
    ASSERT_EQ(2, receiver.read());
    ASSERT_TRUE(receiver.empty());

    EXPECT_CALL(onConsumed, called(4)).Times(1);
    EXPECT_CALL(onConsumed, called(2)).Times(1);
    sender.checkConsumed(onConsumed);
    sender.write(5);
    EXPECT_CALL(onConsumed, called(_)).Times(0);
    sender.checkConsumed(onConsumed);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(5, receiver.read());
    EXPECT_CALL(onConsumed, called(5)).Times(1);
    sender.checkConsumed(onConsumed);
    ASSERT_TRUE(receiver.empty());
}

TEST_F(SpSc, consumed_notification_modify)
{
    OnConsumed onConsumed;
    using IntQueueConsumed = util::spsc::Queue<uint16_t, 6, true>;
    IntQueueConsumed queue;
    IntQueueConsumed::Sender sender(queue);
    IntQueueConsumed::Receiver receiver(queue);
    sender.write(4);
    ASSERT_FALSE(receiver.empty());
    uint16_t& v = receiver.peek();
    ASSERT_EQ(4, v);
    v = v * 2;
    receiver.advance();
    ASSERT_TRUE(receiver.empty());
    EXPECT_CALL(onConsumed, called(8)).Times(1);
    sender.checkConsumed(onConsumed);
}

TEST_F(
    SpSc, a_queue_with_consumed_notification_stays_full_until_the_data_consumption_has_been_acked)
{
    OnConsumed onConsumed;
    using IntQueueConsumed = util::spsc::Queue<uint16_t, 6, true>;

    IntQueueConsumed queue;
    IntQueueConsumed::Sender sender(queue);
    IntQueueConsumed::Receiver receiver(queue);

    sender.write(4);
    sender.write(2);
    sender.write(2);
    sender.write(2);
    sender.write(2);
    sender.write(2);
    ASSERT_TRUE(sender.full());
    receiver.read();
    ASSERT_TRUE(sender.full());

    sender.checkConsumed(onConsumed);
    ASSERT_FALSE(sender.full());
}

TEST_F(SpSc, queue_with_track_consumed_can_be_reset)
{
    OnConsumed onConsumed;
    using IntQueueConsumed = util::spsc::Queue<uint16_t, 6, true>;

    IntQueueConsumed queue;
    IntQueueConsumed::Sender sender(queue);
    IntQueueConsumed::Receiver receiver(queue);

    // Apart from these 3 lines this test is a copy of the consumed_notification one
    sender.write(8);
    sender.write(6);
    queue.reset();

    sender.write(4);
    sender.write(2);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(4, receiver.read());
    ASSERT_EQ(2, receiver.read());
    ASSERT_TRUE(receiver.empty());

    EXPECT_CALL(onConsumed, called(4)).Times(1);
    EXPECT_CALL(onConsumed, called(2)).Times(1);
    sender.checkConsumed(onConsumed);
    sender.write(5);
    EXPECT_CALL(onConsumed, called(_)).Times(0);
    sender.checkConsumed(onConsumed);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(5, receiver.read());
    EXPECT_CALL(onConsumed, called(5)).Times(1);
    sender.checkConsumed(onConsumed);
    ASSERT_TRUE(receiver.empty());
}

TEST_F(SpSc, can_be_reset)
{
    using IntQueueConsumed = util::spsc::Queue<uint16_t, 6>;

    IntQueueConsumed queue;
    IntQueueConsumed::Sender sender(queue);
    IntQueueConsumed::Receiver receiver(queue);

    // Apart from these 3 lines this test is a copy of the consumed_notification one
    sender.write(8);
    sender.write(6);
    queue.reset();

    sender.write(4);
    sender.write(2);
    ASSERT_FALSE(receiver.empty());
    ASSERT_EQ(4, receiver.read());
    ASSERT_EQ(2, receiver.read());
    ASSERT_TRUE(receiver.empty());
}

TEST(SpSc_read_write, can_instantiate_queue_reader_writer)
{
    using Q = util::spsc::Queue<uint16_t, 6, true>;
    Q q;
    util::spsc::QueueReader<Q> reader(q);
    util::spsc::QueueWriter<Q> writer(q);
}

} // namespace
