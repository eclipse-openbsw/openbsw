// Copyright 2024 Accenture.

#include "io/VariantQueue.h"

#include <estd/big_endian.h>
#include <estd/memory.h>
#include <estd/slice.h>

#include <gmock/gmock.h>

#include <cstdint>

namespace
{
struct A
{
    uint8_t values[5];
};

struct __attribute__((packed)) B
{
    ::estd::be_uint16_t x;
    ::estd::be_uint32_t y;
};

struct C
{};

using namespace testing;

constexpr size_t QUEUE_CAPACITY = 40;

using abc_variant_q_type_list = ::io::make_variant_queue<
    ::io::VariantQueueType<A, 3>,
    ::io::VariantQueueType<B>,
    ::io::VariantQueueType<C, 15>>;

static_assert(
    std::is_same<abc_variant_q_type_list::type_list, ::estd::make_type_list<A, B, C>::type>::value,
    "type list from the typedef should be A, B, C");

static_assert(abc_variant_q_type_list::queue_max_element_type == 16, "");

using abc_queue = ::io::variant_q<abc_variant_q_type_list::type_list>;

using Queue = ::io::VariantQueue<abc_variant_q_type_list, QUEUE_CAPACITY>;

struct Visit
{
    ::estd::variant<A, B, C> value = C();

    template<typename T>
    void operator()(T const& v)
    {
        value = v;
    }
};

struct VisitWithPayload
{
    ::estd::variant<A, B, C> value = C();
    ::estd::slice<uint8_t const> payload;

    template<typename T>
    void operator()(T const& v, ::estd::slice<uint8_t const> p)
    {
        value   = v;
        payload = p;
    }
};

TEST(VariantQueue, read_write_no_payload)
{
    Queue queue;
    ::io::MemoryQueueReader<Queue> reader(queue);
    ::io::MemoryQueueWriter<Queue> writer(queue);

    Visit visitor;

    ASSERT_TRUE(abc_queue::write(writer, A{{0, 1, 2, 3, 4}}));
    ASSERT_TRUE(abc_queue::write(
        writer, B{::estd::be_uint16_t ::make(3), ::estd::be_uint32_t::make(0xFAAF1253)}));
    ASSERT_TRUE(abc_queue::write(writer, C{}));
    ASSERT_TRUE(abc_queue::write(writer, C{}));
    ASSERT_FALSE(abc_queue::write(writer, C{})); // out of space

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read(visitor, reader.peek());
    EXPECT_THAT(visitor.value, VariantWith<A>(Field(&A::values, ElementsAre(0, 1, 2, 3, 4))));
    reader.release();

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read(visitor, reader.peek());
    EXPECT_THAT(
        visitor.value, VariantWith<B>(AllOf(Field(&B::x, Eq(3)), Field(&B::y, Eq(0xFAAF1253)))));
    reader.release();

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read(visitor, reader.peek());
    EXPECT_TRUE(visitor.value.is<C>());
    reader.release();
    reader.release(); // one more C element in the queue

    EXPECT_EQ(0, reader.peek().size());
}

TEST(VariantQueue, read_write_with_payload)
{
    Queue queue;
    ::io::MemoryQueueReader<Queue> reader(queue);
    ::io::MemoryQueueWriter<Queue> writer(queue);

    VisitWithPayload visitor;

    uint8_t const payload[] = {0x33, 0x77, 0x99};
    ASSERT_TRUE(abc_queue::write(writer, A{{9, 8, 7, 6, 5}}, payload));

    ASSERT_TRUE(abc_queue::write(
        writer, C{}, 15, [](::estd::slice<uint8_t> buffer) { ::estd::memory::set(buffer, 0xAB); }));

    ASSERT_FALSE(
        abc_queue::write(writer, C{}, 10, [](::estd::slice<uint8_t> buffer) {})); // out of space

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read_with_payload(visitor, reader.peek());
    EXPECT_THAT(visitor.value, VariantWith<A>(Field(&A::values, ElementsAre(9, 8, 7, 6, 5))));
    EXPECT_THAT(visitor.payload, ElementsAreArray(payload));
    reader.release();

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read_with_payload(visitor, reader.peek());
    EXPECT_TRUE(visitor.value.is<C>());
    EXPECT_THAT(visitor.payload, AllOf(SizeIs(15), Each(Eq(0xAB))));
    reader.release();

    EXPECT_EQ(0, reader.peek().size());
}

TEST(VariantQueue, manually_allocate_header)
{
    Queue queue;
    ::io::MemoryQueueWriter<Queue> writer(queue);
    ::io::MemoryQueueReader<Queue> reader(queue);

    Visit visitor;

    auto const b = abc_queue::alloc_header<B>(writer);
    ASSERT_NE(b, nullptr);
    b->x = 42;
    b->y = 1337;
    writer.commit();

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read(visitor, reader.peek());
    EXPECT_THAT(visitor.value, VariantWith<B>(AllOf(Field(&B::x, Eq(42)), Field(&B::y, Eq(1337)))));
    reader.release();

    ASSERT_NE(abc_queue::alloc_header<A>(writer), nullptr);
    writer.commit();
    ASSERT_NE(abc_queue::alloc_header<A>(writer), nullptr);
    writer.commit();
    ASSERT_EQ(abc_queue::alloc_header<A>(writer), nullptr); // out of space
}

TEST(VariantQueue, manually_allocate_payload)
{
    Queue queue;
    ::io::MemoryQueueWriter<Queue> writer(queue);
    ::io::MemoryQueueReader<Queue> reader(queue);

    VisitWithPayload visitor;

    uint8_t const payload[] = {0x33, 0x77, 0x99};

    auto a_payload = abc_queue::alloc_payload(writer, A{{9, 8, 7, 6, 5}}, sizeof(payload));
    ASSERT_NE(0, a_payload.size());
    ::estd::memory::copy(a_payload, payload);
    writer.commit();

    ASSERT_EQ(15U, abc_queue::alloc_payload(writer, C{}, 15).size());
    writer.commit();
    ASSERT_EQ(0U, abc_queue::alloc_payload(writer, C{}, 15).size()); // out of space

    ASSERT_NE(0U, reader.peek().size());
    abc_queue::read_with_payload(visitor, reader.peek());
    EXPECT_THAT(visitor.value, VariantWith<A>(Field(&A::values, ElementsAre(9, 8, 7, 6, 5))));
    EXPECT_THAT(visitor.payload, ElementsAreArray(payload));
    reader.release();
}

} // namespace
