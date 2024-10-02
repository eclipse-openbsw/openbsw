// Copyright 2024 Accenture.

#include "estd/memory.h"

#include "estd/none.h"
#include "internal/TestClassCalls.h"

#include <gmock/gmock.h>

#include <memory>

namespace
{
using namespace ::testing;

struct AllocMock
{
    MOCK_CONST_METHOD1(alloc, ::estd::slice<uint8_t>(size_t));

    ::estd::slice<uint8_t> operator()(size_t n) const { return alloc(n); }
};

struct A
{
    A() : x(1), y(2), z(3) {}

    uint8_t x;
    uint8_t y;
    uint8_t z;
};

TEST(Memory, as_bytes_const)
{
    A const a;

    ::estd::slice<uint8_t const, sizeof(A)> s = ::estd::memory::as_bytes(&a);
    EXPECT_EQ(static_cast<void const*>(&a), static_cast<void const*>(s.data()));
}

TEST(Memory, as_bytes)
{
    A a;

    ::estd::slice<uint8_t const, sizeof(A)> s = ::estd::memory::as_bytes(&a);
    EXPECT_EQ(static_cast<void const*>(&a), static_cast<void const*>(s.data()));
}

TEST(Memory, unaligned)
{
    A a;

    ::estd::memory::unaligned<A> u = ::estd::memory::unaligned<A>::make(a);

    EXPECT_THAT(u.bytes, ElementsAre(1, 2, 3));

    a.x = 5;
    a.y = 6;
    a.z = 7;

    EXPECT_THAT(u.bytes, ElementsAre(1, 2, 3));

    u.set(a);
    EXPECT_THAT(u.bytes, ElementsAre(5, 6, 7));

    A b = u.get();

    EXPECT_EQ(5, b.x);
    EXPECT_EQ(6, b.y);
    EXPECT_EQ(7, b.z);
}

TEST(Memory, as_bytes_for_byte_array)
{
    uint8_t data[10]         = {0x04, 0x03, 0x02, 0x01, 0x04, 0x03, 0x02, 0x01, 0xff, 0xfe};
    uint8_t* pData           = &data[0];
    ::estd::slice<uint8_t> s = ::estd::memory::as_bytes(pData, 10);
    EXPECT_THAT(s, ElementsAreArray(data));
}

TEST(Memory, as_bytes_for_const_byte_array)
{
    uint8_t data[10]               = {0x04, 0x03, 0x02, 0x01, 0x04, 0x03, 0x02, 0x01, 0xff, 0xfe};
    uint8_t const* const pData     = &data[0];
    ::estd::slice<uint8_t const> s = ::estd::memory::as_bytes(pData, 10);
    EXPECT_THAT(s, ElementsAreArray(data));
}

TEST(Memory, as_slice_for_array_of_non_bytes)
{
    uint32_t data[3]          = {0x11111111, 0x11111112, 0x11111113};
    uint32_t* pData           = &data[0];
    ::estd::slice<uint32_t> s = ::estd::memory::as_slice(pData, 3);
    EXPECT_THAT(s, ElementsAreArray(data));
}

TEST(Memory, as_slice_for_const_array_of_non_bytes)
{
    uint32_t data[3]                = {0x11111111, 0x11111112, 0x11111113};
    uint32_t const* const pData     = &data[0];
    ::estd::slice<uint32_t const> s = ::estd::memory::as_slice(pData, 3);
    EXPECT_THAT(s, ElementsAreArray(data));
}

TEST(Memory, as_slice)
{
    A a;

    ::estd::slice<A> s = ::estd::memory::as_slice(&a);
    EXPECT_EQ(&a, s.data());
    EXPECT_EQ(1U, s.size());
}

TEST(Memory, to_bytes)
{
    A const a;
    ::estd::array<uint8_t, sizeof(A)> arr = ::estd::memory::to_bytes<A>(a);

    EXPECT_THAT(arr, ElementsAre(a.x, a.y, a.z));
}

TEST(Memory, destruct)
{
    ::internal::TestClassCalls::reset();
    uint8_t buffer[sizeof(::internal::TestClassCalls)];
    new (buffer)::internal::TestClassCalls();
    EXPECT_EQ(1, ::internal::TestClassCalls::getConstructorCalls());
    EXPECT_EQ(0, ::internal::TestClassCalls::getDestructorCalls());

    ::estd::slice<::internal::TestClassCalls> sl
        = ::estd::make_slice(buffer).reinterpret_as<::internal::TestClassCalls>();

    ::estd::slice<uint8_t> buf = ::estd::memory::destruct(sl);

    EXPECT_EQ(buffer, buf.data());
    EXPECT_EQ(sizeof(::internal::TestClassCalls), buf.size());

    EXPECT_EQ(1, ::internal::TestClassCalls::getConstructorCalls());
    EXPECT_EQ(1, ::internal::TestClassCalls::getDestructorCalls());
}

TEST(Memory, emplace)
{
    alignas(4)::estd::array<uint8_t, 5> a{};
    ::estd::slice<uint8_t> s(a);
    ::estd::memory::emplace<uint32_t>(s).construct(27);

    EXPECT_EQ(4U, s.data() - a.data());
    EXPECT_EQ(1U, s.size());
    EXPECT_EQ(27U, *reinterpret_cast<uint32_t*>(a.data()));

    EXPECT_FALSE(::estd::memory::emplace<uint32_t>(s).has_memory());
}

TEST(Memory, set)
{
    uint8_t destination[5] = {0};
    ::estd::memory::set(destination, 0xAA);

    EXPECT_THAT(destination, Each(0xAA));
}

TEST(Memory, reverse_bytes_even_size)
{
    uint8_t buf[10] = {0x11, 0x21, 0x12, 0x32, 0x23, 0x43, 0x34, 0x54, 0x45, 0x65};
    ::estd::memory::reverse_bytes(buf);
    EXPECT_THAT(buf, ElementsAre(0x65, 0x45, 0x54, 0x34, 0x43, 0x23, 0x32, 0x12, 0x21, 0x11));
}

TEST(Memory, reverse_bytes_odd_size)
{
    uint8_t buf[7] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    ::estd::memory::reverse_bytes(buf);
    EXPECT_THAT(buf, ElementsAre(0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11));
}

TEST(Memory, alloc_and_construct)
{
    AllocMock allocMock;

    uint8_t buf[5 * sizeof(A)];

    EXPECT_CALL(allocMock, alloc(5 * sizeof(A)))
        .Times(1)
        .WillOnce(Return(::estd::slice<uint8_t>(buf)));

    ::estd::slice<A> as;
    ::estd::memory::alloc_and_construct(allocMock, as, 5);
    EXPECT_EQ(1, as[0].x);
    EXPECT_EQ(2, as[0].y);
    EXPECT_EQ(3, as[0].z);
    EXPECT_EQ(5U, as.size());
    EXPECT_EQ(buf, as.reinterpret_as<uint8_t>().data());
}

TEST(Memory, alloc_and_construct_fail)
{
    AllocMock allocMock;

    EXPECT_CALL(allocMock, alloc(5 * sizeof(A)))
        .Times(1)
        .WillOnce(Return(::estd::slice<uint8_t>()));
    ::estd::slice<A> as;
    ::estd::memory::alloc_and_construct(allocMock, as, 5);
    EXPECT_EQ(0U, as.size());
}

TEST(Memory, copy)
{
    uint8_t const source[]  = {1, 2, 3};
    uint8_t destination[10] = {0};

    ::estd::slice<uint8_t> r = ::estd::memory::copy(destination, source);

    EXPECT_EQ(1, destination[0]);
    EXPECT_THAT(r, ElementsAre(1, 2, 3));

    EXPECT_EQ(destination, r.data());
    EXPECT_EQ(3U, r.size());
}

TEST(Memory, copy_returns_empty_if_destination_is_smaller_than_source)
{
    uint8_t const source[] = {1, 2, 3};
    uint8_t destination[2] = {0};

    ::estd::slice<uint8_t> r = ::estd::memory::copy(destination, source);
    EXPECT_EQ(0U, r.size());
}

TEST(Memory, copy_returns_empty_if_source_data_is_nullpointer)
{
    ::estd::slice<uint8_t> const source = {};
    uint8_t destination[2]              = {0};

    EXPECT_EQ(nullptr, source.data());
    ::estd::slice<uint8_t> r = ::estd::memory::copy(destination, source);
    EXPECT_EQ(0U, r.size());
}

TEST(Memory, copy_returns_empty_if_destination_data_is_nullpointer)
{
    uint8_t source[2]                        = {0};
    ::estd::slice<uint8_t> const destination = {};

    EXPECT_EQ(nullptr, destination.data());
    ::estd::slice<uint8_t> r = ::estd::memory::copy(destination, source);
    EXPECT_EQ(0U, r.size());
}

TEST(Memory, move)
{
    uint8_t source[] = {1, 2, 3, 0, 0};
    ::estd::slice<uint8_t> s(source);

    ::estd::slice<uint8_t> r = ::estd::memory::move<uint8_t>(s.offset(3), s.subslice(2));

    EXPECT_THAT(source, ElementsAre(1, 2, 3, 1, 2));
    EXPECT_THAT(r, ElementsAre(1, 2));
}

TEST(Memory, move_overlapping)
{
    uint8_t source[] = {1, 2, 3, 4, 5};
    ::estd::slice<uint8_t> s(source);

    ::estd::slice<uint8_t> r = ::estd::memory::move<uint8_t>(s.offset(1), s.subslice(4));

    EXPECT_THAT(source, ElementsAre(1, 1, 2, 3, 4));
    EXPECT_THAT(r, ElementsAre(1, 2, 3, 4));
}

TEST(Memory, compare)
{
    uint8_t a[] = {1, 2, 3, 4, 5};
    uint8_t b[] = {1, 2, 3, 4, 5};

    EXPECT_EQ(0, ::estd::memory::compare(a, b));
    EXPECT_GT(0, ::estd::memory::compare(a, ::estd::make_slice(b).subslice(3)));

    b[3] = 9;
    EXPECT_GT(0, ::estd::memory::compare(a, b));
}

TEST(Memory, is_equal)
{
    uint8_t const a[] = {1, 2, 3, 4, 5};
    uint8_t const b[] = {1, 2, 3, 4, 5};
    uint8_t const c[] = {1, 2, 2, 4, 5};
    uint8_t const d[] = {1, 2, 2, 4};

    EXPECT_TRUE(::estd::memory::is_equal(a, b));
    EXPECT_FALSE(::estd::memory::is_equal(a, c));
    EXPECT_FALSE(::estd::memory::is_equal(b, c));
    EXPECT_FALSE(::estd::memory::is_equal(c, d));
    EXPECT_FALSE(::estd::memory::is_equal(d, c));
    EXPECT_FALSE(::estd::memory::is_equal(d, ::estd::none));
    EXPECT_FALSE(::estd::memory::is_equal(::estd::none, a));
    EXPECT_TRUE(::estd::memory::is_equal(::estd::none, ::estd::none));
}

TEST(Memory, starts_with)
{
    uint8_t const a[] = {1, 2, 3, 4, 5};
    uint8_t const b[] = {1, 2};
    uint8_t const c[] = {1, 3};

    EXPECT_TRUE(::estd::memory::starts_with(a, b));
    EXPECT_FALSE(::estd::memory::starts_with(b, a));
    EXPECT_FALSE(::estd::memory::starts_with(a, c));
    EXPECT_FALSE(::estd::memory::starts_with(c, a));

    EXPECT_TRUE(::estd::memory::starts_with(::estd::none, ::estd::none));
    EXPECT_TRUE(::estd::memory::starts_with(a, ::estd::none));
    EXPECT_FALSE(::estd::memory::starts_with(::estd::none, c));
}

TEST(Memory, ends_with)
{
    uint8_t const a[] = {1, 2, 3, 4, 5};
    uint8_t const b[] = {4, 5};
    uint8_t const c[] = {1, 3};

    EXPECT_TRUE(::estd::memory::ends_with(a, b));
    EXPECT_FALSE(::estd::memory::ends_with(b, a));
    EXPECT_FALSE(::estd::memory::ends_with(a, c));
    EXPECT_FALSE(::estd::memory::ends_with(c, a));

    EXPECT_TRUE(::estd::memory::ends_with(::estd::none, ::estd::none));
    EXPECT_TRUE(::estd::memory::ends_with(c, ::estd::none));
    EXPECT_FALSE(::estd::memory::ends_with(::estd::none, a));
}

/**
 * \desc
 * Use take() (non-const version) to retrieve single elements
 * and verify that the source slice is advanced accordingly.
 */
TEST(Memory, take_single)
{
    int data[5]                      = {10, 21, 32, 43, 54};
    ::estd::slice<uint8_t> byteSlice = ::estd::make_slice(data).reinterpret_as<uint8_t>();

    EXPECT_EQ(10, ::estd::memory::take<int>(byteSlice));
    EXPECT_EQ(21, ::estd::memory::take<int>(byteSlice));

    ::estd::memory::take<int>(byteSlice) = 9999;
    EXPECT_THAT(data, ElementsAre(10, 21, 9999, 43, 54));

    EXPECT_EQ(reinterpret_cast<uint8_t const*>(&data[3]), byteSlice.data());
    EXPECT_EQ(sizeof(int) * 2U, byteSlice.size());
}

TEST(Memory, align)
{
    uint64_t data[5]                 = {10, 21, 32, 43, 54};
    ::estd::slice<uint8_t> byteSlice = ::estd::make_slice(data).reinterpret_as<uint8_t>();
    auto const orig                  = byteSlice;

    ::estd::memory::align(1, byteSlice);
    EXPECT_EQ(orig.data(), byteSlice.data());

    ::estd::memory::align(2, byteSlice);
    EXPECT_EQ(orig.data(), byteSlice.data());

    ::estd::memory::align(4, byteSlice);
    EXPECT_EQ(orig.data(), byteSlice.data());

    ::estd::memory::align(8, byteSlice);
    EXPECT_EQ(orig.data(), byteSlice.data());

    byteSlice.advance(1);

    ::estd::memory::align(1, byteSlice);
    EXPECT_EQ(orig.data() + 1, byteSlice.data());

    ::estd::memory::align(2, byteSlice);
    EXPECT_EQ(orig.data() + 2, byteSlice.data());

    ::estd::memory::align(4, byteSlice);
    EXPECT_EQ(orig.data() + 4, byteSlice.data());

    ::estd::memory::align(8, byteSlice);
    EXPECT_EQ(orig.data() + 8, byteSlice.data());

    byteSlice.advance(1);

    ::estd::memory::align(8, byteSlice);
    EXPECT_EQ(orig.data() + 16, byteSlice.data());
    ::estd::memory::align(4, byteSlice);
    EXPECT_EQ(orig.data() + 16, byteSlice.data());
    ::estd::memory::align(2, byteSlice);
    EXPECT_EQ(orig.data() + 16, byteSlice.data());
    ::estd::memory::align(1, byteSlice);
    EXPECT_EQ(orig.data() + 16, byteSlice.data());
}

/**
 * \desc
 * Use take() (const version) to retrieve single elements
 * and verify that the source slice is advanced accordingly.
 */
TEST(Memory, take_single_const)
{
    int const data[5] = {10, 21, 32, 43, 54};
    ::estd::slice<uint8_t const> byteSlice
        = ::estd::make_slice(data).reinterpret_as<uint8_t const>();

    EXPECT_EQ(10, ::estd::memory::take<int>(byteSlice));
    EXPECT_EQ(21, ::estd::memory::take<int>(byteSlice));
    EXPECT_EQ(32, ::estd::memory::take<int>(byteSlice));

    EXPECT_EQ(reinterpret_cast<uint8_t const*>(&data[3]), byteSlice.data());
    EXPECT_EQ(sizeof(int) * 2U, byteSlice.size());
}

/**
 * \desc
 * Use take() (non-const version) to retrieve slices of elements
 * and verify that the source slice is advanced accordingly.
 */
TEST(Memory, take_slice)
{
    int data[10]                     = {10, 21, 32, 43, 54, 65, 76, 87, 98, 19};
    ::estd::slice<uint8_t> byteSlice = ::estd::make_slice(data).reinterpret_as<uint8_t>();

    EXPECT_THAT(::estd::memory::take<int>(3, byteSlice), ElementsAre(10, 21, 32));
    EXPECT_THAT(::estd::memory::take<int>(4, byteSlice), ElementsAre(43, 54, 65, 76));

    ::estd::slice<int> s = ::estd::memory::take<int>(2, byteSlice);
    ASSERT_EQ(2U, s.size());
    s[0] = 424242;
    s[1] = 1234546789;

    EXPECT_THAT(data, ElementsAre(10, 21, 32, 43, 54, 65, 76, 424242, 1234546789, 19));

    EXPECT_EQ(reinterpret_cast<uint8_t const*>(&data[9]), byteSlice.data());
    EXPECT_EQ(sizeof(int), byteSlice.size());
}

/**
 * \desc
 * Attempt to take() (non-const version) more elements than are available in the source slice.
 * Verify that an empty slice is returned and that the source slice is empty afterwards.
 */
TEST(Memory, take_slice_more_than_size)
{
    int data[2]                      = {10, 21};
    ::estd::slice<uint8_t> byteSlice = ::estd::make_slice(data).reinterpret_as<uint8_t>();

    ::estd::slice<int> s = ::estd::memory::take<int>(3, byteSlice);
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(byteSlice.size(), 0);
}

/**
 * \desc
 * Use take() (const version) to retrieve slices of elements
 * and verify that the source slice is advanced accordingly.
 */
TEST(Memory, take_slice_const)
{
    int const data[10] = {10, 21, 32, 43, 54, 65, 76, 87, 98, 19};
    ::estd::slice<uint8_t const> byteSlice
        = ::estd::make_slice(data).reinterpret_as<uint8_t const>();

    EXPECT_THAT(::estd::memory::take<int>(3, byteSlice), ElementsAre(10, 21, 32));
    EXPECT_THAT(::estd::memory::take<int>(4, byteSlice), ElementsAre(43, 54, 65, 76));
    EXPECT_THAT(::estd::memory::take<int>(2, byteSlice), ElementsAre(87, 98));

    EXPECT_EQ(reinterpret_cast<uint8_t const*>(&data[9]), byteSlice.data());
    EXPECT_EQ(sizeof(int), byteSlice.size());
}

/**
 * \desc
 * Attempt to take() (const version) more elements than are available in the source slice.
 * Verify that an empty slice is returned and that the source slice is empty afterwards.
 */
TEST(Memory, take_slice_const_more_than_size)
{
    int const data[2] = {10, 21};
    ::estd::slice<uint8_t const> byteSlice
        = ::estd::make_slice(data).reinterpret_as<uint8_t const>();

    ::estd::slice<int const> s = ::estd::memory::take<int const>(3, byteSlice);
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(byteSlice.size(), 0);
}

TEST(Memory, take_or)
{
    uint8_t const data[] = {1};
    ::estd::slice<uint8_t const> s(data);

    EXPECT_EQ(1U, ::estd::memory::take_or<uint8_t>(s, 2U));
    EXPECT_EQ(2U, ::estd::memory::take_or<uint8_t>(s, 2U));
}

uint8_t default_to_2() { return 2U; }

TEST(Memory, peek_or_else)
{
    uint8_t const data[] = {1};
    ::estd::slice<uint8_t const> s(data);

    EXPECT_EQ(1U, ::estd::memory::peek_or_else<uint8_t>(s, default_to_2));
    EXPECT_EQ(1U, ::estd::memory::peek_or_else<uint8_t>(s, default_to_2));
    s.advance(1U);
    EXPECT_EQ(2U, ::estd::memory::peek_or_else<uint8_t>(s, default_to_2));
}

TEST(Memory, take_or_else)
{
    uint8_t const data[] = {1};
    ::estd::slice<uint8_t const> s(data);

    EXPECT_EQ(1U, ::estd::memory::take_or_else<uint8_t>(s, default_to_2));
    EXPECT_EQ(2U, ::estd::memory::take_or_else<uint8_t>(s, default_to_2));
}

TEST(Memory, make_primitive)
{
    ::estd::array<uint8_t, 100U> buffer;
    buffer.fill(0xAA);

    ::estd::slice<uint8_t> s(buffer);
    EXPECT_THAT(::estd::memory::make<uint32_t>(3, s), ElementsAre(0U, 0U, 0U));
    EXPECT_EQ(100 - 3 * sizeof(uint32_t), s.size());
}

TEST(Memory, make_with_user_constructor)
{
    ::estd::array<uint8_t, 100U> buffer;
    buffer.fill(0xFF);

    ::estd::slice<uint8_t> s(buffer);
    ::estd::slice<A> a = ::estd::memory::make<A>(5, s);
    EXPECT_EQ(5U, a.size());
    EXPECT_THAT(a, Each(AllOf(Field(&A::x, Eq(1U)), Field(&A::y, Eq(2U)), Field(&A::z, Eq(3U)))));
    EXPECT_EQ(100 - 5 * sizeof(A), s.size());
}

TEST(Memory, make_buffer_too_small)
{
    ::estd::array<uint8_t, 10U> buffer;
    buffer.fill(0xFF);

    ::estd::slice<uint8_t> s(buffer);
    EXPECT_THAT(::estd::memory::make<uint32_t>(300, s), SizeIs(0U));

    EXPECT_EQ(10U, s.size());
    EXPECT_THAT(buffer, Each(Eq(0xFFU)));
}

TEST(Memory, slice_split)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t> s(data);

    struct
    {
        ::estd::slice<uint8_t> s1;
        ::estd::slice<uint8_t> s2;
        ::estd::slice<uint8_t> s3;
    } result
        = {::estd::memory::split(s, 1), ::estd::memory::split(s, 2), ::estd::memory::split(s, 3)};

    EXPECT_EQ(1U, result.s1.size());
    EXPECT_THAT(result.s1, ElementsAreArray(&data[0], 1));
    EXPECT_EQ(2U, result.s2.size());
    EXPECT_THAT(result.s2, ElementsAreArray(&data[1], 2));
    EXPECT_EQ(3U, result.s3.size());
    EXPECT_THAT(result.s3, ElementsAreArray(&data[3], 3));
    EXPECT_EQ(4U, s.size());
    EXPECT_THAT(s, ElementsAreArray(&data[6], 4));
}

TEST(Memory, split_leaves_empty_slice_if_split_at_offset_size)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t> s(data);
    ::estd::slice<uint8_t> e = ::estd::memory::split(s, 10);
    EXPECT_EQ(0U, s.size());
    EXPECT_EQ(10U, e.size());
    EXPECT_THAT(e, ElementsAreArray(data));
}

TEST(Memory, split_eaves_slice_untouched_if_split_with_offset_zero)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t> s(data);
    ::estd::slice<uint8_t> e = ::estd::memory::split(s, 0);
    EXPECT_EQ(10U, s.size());
    EXPECT_EQ(0U, e.size());
    EXPECT_THAT(s, ElementsAreArray(data));
}

TEST(Memory, compare_returns_zero_because_both_buffers_are_equal)
{
    uint8_t array1[5] = {0, 1, 2, 3, 4};
    uint8_t array2[5] = {0, 1, 2, 3, 4};

    EXPECT_EQ(0, ::estd::memory::compare(array1, array2));
}

TEST(Memory, compare_returns_a_negative_value_because_the_first_byte_of_array1_is_less)
{
    uint8_t array1[] = {0, 1, 2, 3, 3};
    uint8_t array2[] = {0, 1, 2, 3, 4};

    EXPECT_GT(0, ::estd::memory::compare(array1, array2));
}

TEST(Memory, compare_returns_a_negative_value_because_the_other_buffer_is_longer)
{
    uint8_t array1[] = {0, 1, 2, 3, 4};
    uint8_t array2[] = {0, 1, 2, 3, 4, 5};

    EXPECT_GT(0, ::estd::memory::compare(array2, array1));
}

TEST(Memory, compare_returns_negative_value_because_buffer_is_less)
{
    uint8_t array1[5] = {0, 1, 2, 3, 4};
    uint8_t array2[5] = {0, 1, 2, 3, 6};

    EXPECT_GT(0, ::estd::memory::compare(array1, array2));
}

TEST(Memory, compare_returns_positive_value_because_buffer_is_greater)
{
    uint8_t array1[5] = {0, 1, 2, 3, 7};
    uint8_t array2[5] = {0, 1, 2, 3, 6};

    EXPECT_LT(0, ::estd::memory::compare(array1, array2));
}

} // namespace
