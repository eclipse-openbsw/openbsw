// Copyright 2024 Accenture.

#include <etl/algorithm.h>
#include <etl/unaligned_type.h>
#include <io/VariantQueue.h>

#include <gtest/gtest.h>

#include <cstdio>

namespace
{
// EXAMPLE_START declare

struct A
{
    uint8_t some_bytes[10];
};

struct __attribute__((packed)) B
{
    ::etl::be_uint16_t x;
    ::etl::be_uint32_t y;
};

constexpr size_t MAX_B_PAYLOAD_SIZE = 20;

using MyVariantQTypeList = ::io::make_variant_queue<
    ::io::VariantQueueType<A>, // struct A doesn't need payload
    ::io::VariantQueueType<B, MAX_B_PAYLOAD_SIZE>>;

using MyTypes = MyVariantQTypeList::type_list;

static constexpr size_t TOTAL_QUEUE_CAPACITY = 512;

using MyQueue = ::io::VariantQueue<MyVariantQTypeList, TOTAL_QUEUE_CAPACITY>;

// EXAMPLE_END declare

void write()
{
    // EXAMPLE_START write
    MyQueue queue;
    MyQueue::Writer writer(queue);

    // write struct A without payload:
    A const a = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::io::variant_q<MyTypes>::write(writer, a);

    // write struct B with payload:
    B const b{42, 123456};
    uint8_t const payload[] = {0xAA, 0xBB, 0xCC};
    ::io::variant_q<MyTypes>::write(writer, b, payload);

    // write struct B with payload, but fill it with content in memory allocated from the queue
    // (useful when the payload is too big to make a full copy)
    size_t const big_payload_size = 80U;
    ::io::variant_q<MyTypes>::write(
        writer,
        b,
        big_payload_size,
        [](::etl::span<uint8_t> const& buffer) { ::etl::fill(buffer.begin(), buffer.end(), 42); });
    // EXAMPLE_END write
}

void read_no_payload()
{
    // EXAMPLE_START read_no_payload
    struct Visit
    {
        void operator()(A const& /* a */) { printf("received A"); }

        void operator()(B const& /* b */) { printf("received B"); }
    };

    MyQueue queue;
    MyQueue::Reader reader(queue);

    Visit v;
    while (!reader.empty())
    {
        ::io::variant_q<MyTypes>::read(v, reader.peek());
        reader.release();
    }
    // EXAMPLE_END read_no_payload
}

void read_with_payload()
{
    // EXAMPLE_START read_with_payload
    struct VisitWithPayload
    {
        void operator()(A const& /* a */, ::etl::span<uint8_t const> /* payload */)
        {
            printf("received A");
        }

        void operator()(B const& /* b */, ::etl::span<uint8_t const> /* payload */)
        {
            printf("received B");
        }
    };

    MyQueue queue;
    MyQueue::Reader reader(queue);

    VisitWithPayload vp;
    while (!reader.empty())
    {
        ::io::variant_q<MyTypes>::read_with_payload(vp, reader.peek());
        reader.release();
    }
    // EXAMPLE_END read_with_payload
}

// EXAMPLE_END

TEST(VariantQueue, UsageExamples)
{
    write();
    read_with_payload();
    read_no_payload();
}

} // namespace
