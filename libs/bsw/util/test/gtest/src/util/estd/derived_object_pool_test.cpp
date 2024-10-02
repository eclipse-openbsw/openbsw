// Copyright 2024 Accenture.

#include "util/estd/derived_object_pool.h"

#include <estd/array.h>

#include <gmock/gmock.h>

#include <algorithm>

using namespace ::testing;

namespace
{
uint32_t numberOfA = 0U;
uint32_t numberOfB = 0U;

struct A
{
public:
    A(uint32_t valueOfA) : _valueOfA(valueOfA) { ++numberOfA; }

    virtual ~A() { --numberOfA; }

    uint32_t _valueOfA;
};

struct B : public A
{
public:
    B(uint32_t valueOfA, uint16_t valueOfB) : A(valueOfA), _valueOfB(valueOfB) { ++numberOfB; }

    ~B() override { --numberOfB; }

    uint64_t _valueOfB;
};

struct C : public B
{
public:
    C() : B(0U, 1U) {}

    uint32_t _valueOfC[8];
};

TEST(DerivedObjectPool, Constructor)
{
    ::estd::array<uint8_t, 3 * sizeof(A)> buffer{};
    ::util::estd::block_pool blockPool(buffer.data(), buffer.size(), sizeof(A));
    ::util::estd::derived_object_pool<A> cut(blockPool);
    EXPECT_TRUE(cut.full());
    EXPECT_FALSE(cut.empty());
    EXPECT_EQ(3U, cut.size());
    EXPECT_EQ(3U, cut.max_size());
    EXPECT_EQ(&blockPool, &cut.get_block_pool());
    EXPECT_EQ(
        &blockPool,
        &static_cast<::util::estd::derived_object_pool<A> const*>(&cut)->get_block_pool());
}

TEST(DerivedObjectPool, AcquireAndReleaseObjects)
{
    ::estd::array<uint8_t, 3 * sizeof(B)> buffer{};
    ::util::estd::block_pool blockPool(buffer.data(), buffer.size(), sizeof(B));
    ::util::estd::derived_object_pool<A> cut(blockPool);
    B const* firstBuffer = reinterpret_cast<B const*>(buffer.data());
    numberOfA            = 0U;
    numberOfB            = 0U;
    for (uint8_t loop = 0; loop < 2U; ++loop)
    {
        EXPECT_FALSE(cut.contains(*(firstBuffer - 1)));
        EXPECT_TRUE(cut.contains(*firstBuffer));
        EXPECT_TRUE(cut.contains(*(firstBuffer + 1)));
        EXPECT_TRUE(cut.contains(*(firstBuffer + 2)));
        EXPECT_FALSE(cut.contains(*(firstBuffer + 3)));
        A* objects[3];
        EXPECT_TRUE(cut.full());
        EXPECT_FALSE(cut.empty());
        objects[0] = new (cut.allocate<B>().release()) B(12U, 14U);
        EXPECT_THAT(objects[0], NotNull());
        EXPECT_FALSE(cut.full());
        EXPECT_FALSE(cut.empty());
        objects[1] = new (cut.allocate<A>().release()) A(25U);
        EXPECT_THAT(objects[1], NotNull());
        EXPECT_FALSE(cut.full());
        EXPECT_FALSE(cut.empty());
        objects[2] = new (cut.allocate<B>().release()) B(1U, 3U);
        EXPECT_THAT(objects[2], NotNull());
        EXPECT_FALSE(cut.full());
        EXPECT_TRUE(cut.empty());
        // now we should have 3 As and 2 Bs
        EXPECT_EQ(3U, numberOfA);
        EXPECT_EQ(2U, numberOfB);
        // check whether objects are in order
        ::std::stable_sort(objects, objects + 3U);
        EXPECT_EQ(firstBuffer, objects[0]);
        EXPECT_EQ(firstBuffer + 1, objects[1]);
        EXPECT_EQ(firstBuffer + 2, objects[2]);
        {
            ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
            EXPECT_THROW(cut.allocate<B>(), ::estd::assert_exception);
        }
        // release all objects
        cut.release(*objects[0]);
        EXPECT_FALSE(cut.full());
        EXPECT_FALSE(cut.empty());
        cut.release(*objects[1]);
        EXPECT_FALSE(cut.full());
        EXPECT_FALSE(cut.empty());
        cut.release(*objects[2]);
        EXPECT_TRUE(cut.full());
        EXPECT_FALSE(cut.empty());
        // all object counts should be 0
        EXPECT_EQ(0U, numberOfA);
        EXPECT_EQ(0U, numberOfB);
    }
}

TEST(DerivedObjectPool, AllocateBuffer)
{
    {
        ::util::estd::declare::derived_object_pool<A, 2, A> cut;
        A* objects[2];
        objects[0] = new (cut.allocate<A>().release()) A(25U);
        objects[1] = new (cut.allocate<A>().release()) A(40U);
        // check whether objects are in order
        ::std::stable_sort(objects, objects + 2U);
        EXPECT_EQ(
            static_cast<int32_t>(sizeof(A)),
            (reinterpret_cast<uint8_t const*>(objects[1])
             - reinterpret_cast<uint8_t const*>(objects[0])));
    }
    {
        ::util::estd::declare::derived_object_pool<A, 2, ::estd::type_list<A, ::estd::type_list<B>>>
            cut;
        A* objects[2];
        objects[0] = new (cut.allocate<B>().release()) B(12U, 13U);
        objects[1] = new (cut.allocate<A>().release()) A(40U);
        // check whether objects are in order
        ::std::stable_sort(objects, objects + 2U);
        EXPECT_EQ(
            static_cast<int32_t>(sizeof(B)),
            (reinterpret_cast<uint8_t const*>(objects[1])
             - reinterpret_cast<uint8_t const*>(objects[0])));
    }
}

TEST(DerivedObjectPool, AllocateBufferWithTooBigType)
{
    ::util::estd::declare::derived_object_pool<A, 2, ::estd::type_list<A, ::estd::type_list<B>>>
        cut;
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    EXPECT_THROW(cut.allocate<C>(), ::estd::assert_exception);
    EXPECT_THAT(new (cut.allocate<B>().release()) B(12U, 13U), NotNull());
    EXPECT_THAT(new (cut.allocate<A>().release()) A(40U), NotNull());
}

} // anonymous namespace
