// Copyright 2024 Accenture.

#include "estd/slice.h"

#include "estd/array.h"
#include "estd/gtest_extensions.h"
#include "estd/vector.h"

#include <concepts/iterable.h>

#include <platform/estdint.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>

namespace
{
using namespace ::estd::test;
using namespace ::testing;

struct StaticIntSlice5_P
{
    static size_t const LENGTH = 5;
    int32_t data[LENGTH]       = {0};
    using Subject              = ::estd::slice<int32_t, LENGTH>;
    Subject subject;

    StaticIntSlice5_P() : subject(data)
    {
        for (size_t i = 0; i < LENGTH; ++i)
        {
            data[i] = i + 1;
        }
    }
};

size_t const StaticIntSlice5_P::LENGTH;

INSTANTIATE_TYPED_TEST_SUITE_P(Slice_static, ForwardIterableConcept, StaticIntSlice5_P);
INSTANTIATE_TYPED_TEST_SUITE_P(Slice_static, ReverseIterableConcept, StaticIntSlice5_P);

struct DynamicSlice5_P
{
    enum Constants
    {
        LENGTH = 5
    };

    int32_t data[LENGTH] = {0};
    using Subject        = ::estd::slice<int32_t>;
    Subject subject;

    DynamicSlice5_P() : subject(data)
    {
        for (size_t i = 0; i < LENGTH; ++i)
        {
            data[i] = i + 1;
        }
    }
};

INSTANTIATE_TYPED_TEST_SUITE_P(Slice_dynamic, ForwardIterableConcept, DynamicSlice5_P);
INSTANTIATE_TYPED_TEST_SUITE_P(Slice_dynamic, ReverseIterableConcept, DynamicSlice5_P);

TEST(Slice_static, can_be_constructed_from_a_c_array)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<uint8_t, 5> s5(data);

    EXPECT_EQ(10U, s10.size());
    EXPECT_EQ(5U, s5.size());

    EXPECT_EQ(0U, s10.at<0>());
    EXPECT_EQ(0U, s10.at<0>());

    // should not compile
    /* ::estd::slice<uint8_t, 11> s(data); */
}

TEST(Slice_static, can_be_constructed_from_a_estd_array)
{
    ::estd::array<uint8_t, 10> data = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<uint8_t, 5> s5(data);

    EXPECT_EQ(10U, s10.size());
    EXPECT_EQ(5U, s5.size());

    EXPECT_EQ(0U, s10.at<0>());
    EXPECT_EQ(0U, s10.at<0>());

    // should not compile
    /* ::estd::slice<uint8_t, 11> s(data); */
}

TEST(Slice_static, can_be_constructed_explicitly_from_a_pointer)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t* pointer = data;

    ::estd::slice<uint8_t, 10> s10 = ::estd::slice<uint8_t, 10>::from_pointer(pointer);
    ::estd::slice<uint8_t, 5> s5   = ::estd::slice<uint8_t, 5>::from_pointer(pointer);

    EXPECT_EQ(10U, s10.size());
    EXPECT_EQ(5U, s5.size());
    EXPECT_EQ(pointer, s10.data());
    EXPECT_EQ(pointer, s5.data());
}

TEST(Slice_static, does_compile_time_range_checking)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<uint8_t, 5> s5(data);

    EXPECT_EQ(0U, s10.at<0>());
    EXPECT_EQ(9U, s10.at<9>());

    EXPECT_EQ(0U, s5.at<0>());
    EXPECT_EQ(4U, s5.at<4>());

    // should not compile
    /* EXPECT_EQ(0, s10.at<10>()); */
    /* EXPECT_EQ(0, s5.at<5>()); */
}

TEST(Slice_static, does_runtime_range_checking)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<uint8_t, 5> s5(data);

    EXPECT_EQ(0U, s10[0]);
    EXPECT_EQ(9U, s10[9]);
    EXPECT_EQ(0U, s10.at(0));
    EXPECT_EQ(9U, s10.at(9));

    EXPECT_EQ(0U, s5[0]);
    EXPECT_EQ(4U, s5[4]);
    EXPECT_EQ(0U, s5.at(0));
    EXPECT_EQ(4U, s5.at(4));

    ASSERT_NO_THROW({ s10[10]; });
    ASSERT_NO_THROW({ s5[6]; });

    ASSERT_THROW({ s10.at(10); }, ::estd::assert_exception);
    ASSERT_THROW({ s5.at(6); }, ::estd::assert_exception);
}

TEST(Slice_static, can_be_be_created_from_another_slice_with_a_compile_time_offset)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<uint8_t, 5> s5 = s10.offset<5>();

    EXPECT_EQ(0U, s10.at<0>());
    EXPECT_EQ(9U, s10.at<9>());

    EXPECT_EQ(5U, s5.at<0>());
    EXPECT_EQ(9U, s5.at<4>());

    // should not compile
    /* s10.offset<10>(); */
}

::estd::slice<int32_t, 9> testfunc2(::estd::slice<int32_t, 10> x) { return x.offset<1>(); }

TEST(Slice_static, can_be_passed_into_and_returned_from_a_function)
{
    int32_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<int32_t, 10> s(data);

    ::estd::slice<int32_t, 9> r1 = testfunc2(s);
    EXPECT_EQ(9U, r1.size());

    ::estd::slice<int32_t, 9> r2 = testfunc2(data);
    EXPECT_EQ(9U, r2.size());
}

TEST(Slice_static, can_be_be_created_as_subslice_of_another_slice)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<uint8_t, 5> s5 = s10.subslice<5>();

    EXPECT_EQ(0U, s10.at<0>());
    EXPECT_EQ(9U, s10.at<9>());

    EXPECT_EQ(0U, s5.at<0>());
    EXPECT_EQ(4U, s5.at<4>());
}

TEST(Slice_static, can_be_be_created_as_dynamic_subslice_of_another_slice)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, sizeof(data)> s10(data);
    ::estd::slice<uint8_t> dynamic_subslice = s10.subslice(5);

    EXPECT_EQ(0, s10[0]);
    EXPECT_EQ(9, s10[9]);

    EXPECT_EQ(0, dynamic_subslice[0]);
    EXPECT_EQ(4, dynamic_subslice[4]);

    EXPECT_EQ(0U, s10.subslice(11).size());
}

struct SimpleStruct
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
};

TEST(Slice_static, can_reinterpret_its_data_as_another_type)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t, 10> s10(data);
    ::estd::slice<SimpleStruct, 2> ss2 = ::estd::reinterpret_as<SimpleStruct>(s10);

    EXPECT_EQ(0, ss2.at<0>().a);
    EXPECT_EQ(1, ss2.at<0>().b);
    EXPECT_EQ(2, ss2.at<0>().c);
    EXPECT_EQ(3, ss2.at<0>().d);

    EXPECT_EQ(4, ss2.at<1>().a);
    EXPECT_EQ(5, ss2.at<1>().b);
    EXPECT_EQ(6, ss2.at<1>().c);
    EXPECT_EQ(7, ss2.at<1>().d);

    static_assert(4 == sizeof(SimpleStruct), "");

    ::estd::slice<SimpleStruct, 1> ss1
        = ::estd::reinterpret_as<SimpleStruct>(s10.offset<6>()).subslice<1>();

    EXPECT_EQ(6, ss1.at<0>().a);
    EXPECT_EQ(7, ss1.at<0>().b);
    EXPECT_EQ(8, ss1.at<0>().c);
    EXPECT_EQ(9, ss1.at<0>().d);
}

TEST(Slice_dynamic, can_be_constructed_from_vector)
{
    ::estd::declare::vector<int32_t, 10> v1;
    ::estd::slice<int32_t> s1(v1);
    EXPECT_EQ(v1.data(), s1.data());
    EXPECT_EQ(v1.size(), s1.size());

    ::estd::declare::vector<int32_t, 20> v2;
    ::estd::vector<int32_t>& v3 = v2;
    ::estd::slice<int32_t> s3(v3);
    EXPECT_EQ(v3.data(), s3.data());
    EXPECT_EQ(v3.size(), s3.size());
}

TEST(Slice_dynamic, can_be_constructed_from_a_c_array)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t> s10(data);
    EXPECT_EQ(10U, s10.size());
    EXPECT_EQ(data, s10.data());
}

TEST(Slice_dynamic, can_be_constructed_from_something_that_has_data_and_size)
{
    ::estd::array<uint8_t, 10> data;
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[3] = 3;
    data[4] = 4;
    data[5] = 5;
    data[6] = 6;
    data[7] = 7;
    data[8] = 8;
    data[9] = 9;

    ::estd::slice<uint8_t> s10(data);
    EXPECT_EQ(10U, s10.size());
}

TEST(Slice_dynamic, can_be_constructed_from_a_temporary_something_that_has_data_and_size)
{
    ::estd::slice<uint8_t const> s10((::estd::array<uint8_t, 10>()));
    EXPECT_EQ(10U, s10.size());
}

TEST(Slice_dynamic, does_runtime_range_checking)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t> s10(data);

    EXPECT_EQ(0, s10[0]);
    EXPECT_EQ(9, s10[9]);
    EXPECT_EQ(0, s10.at(0));
    EXPECT_EQ(9, s10.at(9));

    ASSERT_NO_THROW({ s10[10]; });
    ASSERT_THROW({ s10.at(10); }, ::estd::assert_exception);
}

TEST(Slice_dynamic, can_be_constructed_explicitly_from_a_pointer)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t* pointer = data;

    ::estd::slice<uint8_t> s10 = ::estd::slice<uint8_t>::from_pointer(pointer, 10);

    EXPECT_EQ(10U, s10.size());
}

TEST(Slice_dynamic, can_be_be_created_from_another_slice_with_an_offset)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t> s10(data);
    ::estd::slice<uint8_t> s5 = s10.offset(5);

    EXPECT_EQ(0, s10[0]);
    EXPECT_EQ(9, s10[9]);

    EXPECT_EQ(5, s5[0]);
    EXPECT_EQ(9, s5[4]);

    EXPECT_EQ(0U, s10.offset(10).size());
    EXPECT_EQ(0U, s10.offset(15).size());
}

TEST(Slice_dynamic, can_be_be_created_as_subslice_of_another_slice)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t> s10(data);
    ::estd::slice<uint8_t> s5 = s10.subslice(5);
    ASSERT_EQ(5U, s5.size());

    EXPECT_EQ(0U, s10[0]);
    EXPECT_EQ(9U, s10[9]);

    EXPECT_EQ(0U, s5[0]);
    EXPECT_EQ(4U, s5[4]);

    EXPECT_EQ(0U, s10.subslice(11).size());
}

TEST(Slice_dynamic, advance_by_number_of_bytes_less_than_size)
{
    uint8_t const data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t const> s(data);
    s.advance(1);
    EXPECT_THAT(s, Slice(&data[1], 9));
    s.advance(2);
    EXPECT_THAT(s, Slice(&data[3], 7));
    s.advance(3);
    EXPECT_THAT(s, Slice(&data[6], 4));
}

TEST(Slice_dynamic, makes_zero_sized_slice_if_advance_too_large)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t> s(data);
    s.advance(s.size() + 1);
    EXPECT_EQ(0U, s.size());
}

TEST(Slice_dynamic, leaves_slice_untouched_if_advance_with_offset_zero)
{
    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<uint8_t> s(data);
    s.advance(0);
    EXPECT_EQ(10U, s.size());
    EXPECT_THAT(s, ElementsAreArray(data));
}

TEST(Slice_dynamic, can_be_trimmed)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t> s10(data);
    ::estd::slice<uint8_t> s5(s10);
    EXPECT_EQ(5U, s5.trim(5).size());
    ASSERT_EQ(5U, s5.size());

    EXPECT_EQ(0U, s10[0]);
    EXPECT_EQ(9U, s10[9]);

    EXPECT_EQ(0U, s5[0]);
    EXPECT_EQ(4U, s5[4]);

    s10.trim(11);
    EXPECT_EQ(0U, s10.size());
}

TEST(Slice_dynamic, can_reinterpret_its_data_as_another_type)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    uint8_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    ::estd::slice<uint8_t> s10(data);
    ::estd::slice<SimpleStruct> ss2 = ::estd::reinterpret_as<SimpleStruct>(s10);

    EXPECT_EQ(0U, ss2[0].a);
    EXPECT_EQ(1U, ss2[0].b);
    EXPECT_EQ(2U, ss2[0].c);
    EXPECT_EQ(3U, ss2[0].d);

    EXPECT_EQ(4U, ss2[1].a);
    EXPECT_EQ(5U, ss2[1].b);
    EXPECT_EQ(6U, ss2[1].c);
    EXPECT_EQ(7U, ss2[1].d);

    static_assert(4 == sizeof(SimpleStruct), "");

    ::estd::slice<SimpleStruct> ss1
        = ::estd::reinterpret_as<SimpleStruct>(s10.offset(6)).subslice(1);

    EXPECT_EQ(6U, ss1[0].a);
    EXPECT_EQ(7U, ss1[0].b);
    EXPECT_EQ(8U, ss1[0].c);
    EXPECT_EQ(9U, ss1[0].d);

    EXPECT_EQ(0U, s10.subslice(3).reinterpret_as<SimpleStruct>().size());
}

::estd::slice<int32_t> testfunc(::estd::slice<int32_t> x) { return x.offset(1); }

TEST(Slice_dynamic, can_be_passed_into_and_returned_from_a_function)
{
    int32_t data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ::estd::slice<int32_t> s(data);
    ::estd::slice<int32_t> const cs(data);

    ::estd::slice<int32_t> r1 = testfunc(s);
    EXPECT_EQ(9U, r1.size());

    ::estd::slice<int32_t> r2 = testfunc(data);
    EXPECT_EQ(9U, r2.size());

    ::estd::slice<int32_t> r3 = testfunc(cs);
    EXPECT_EQ(9U, r3.size());
}

TEST(Slice_dynamic, can_be_default_constructed)
{
    ::estd::slice<int32_t> s;
    ASSERT_EQ(0U, s.size());
}

TEST(Slice_dynamic, non_const_can_be_converted_to_const)
{
    int32_t a[5];
    ::estd::slice<int32_t> s(a);
    EXPECT_THAT(::estd::slice<int32_t const>(s), Slice(a, 5));
}

TEST(Slice_dynamic, slice_can_be_constructed_from_move)
{
    estd::array<uint8_t, 16> data;
    estd::slice<uint8_t> s1 = std::move(data);
    EXPECT_EQ(16, s1.size());
    estd::slice<uint8_t> s2{std::move(data)};
    EXPECT_EQ(16, s2.size());
}

TEST(Slice_static, non_const_can_be_converted_to_const)
{
    int32_t a[5] = {0};
    ::estd::slice<int32_t, 5> s(a);
    ::estd::slice<int32_t const, 5> c(s);
    EXPECT_EQ(a, c.data());
}

TEST(make_str, can_create_slice_from_string)
{
    char const* const s              = "Hello";
    ::estd::slice<uint8_t const> str = ::estd::make_str(s);

    EXPECT_EQ(s, str.reinterpret_as<char const>().data());
    EXPECT_EQ(strlen(s), str.size());
}

TEST(make_slice, can_create_slices_using_type_inference)
{
    {
        int32_t a[5];
        ::estd::slice<int32_t, 5> s = ::estd::make_static_slice(a);
        EXPECT_EQ(a, s.data());
    }

    {
        ::estd::array<int32_t, 5> a;
        ::estd::slice<int32_t, 5> s = ::estd::make_static_slice(a);
        EXPECT_EQ(a.data(), s.data());
    }

    {
        ::estd::array<int32_t, 5> const a = {{1, 2, 3, 4, 5}};
        ::estd::slice<int32_t const, 5> s = ::estd::make_static_slice(a);
        EXPECT_EQ(a.data(), s.data());
    }

    {
        ::estd::declare::vector<int32_t, 10> v;
        v.push_back(0);
        v.push_back(0);
        v.push_back(0);
        EXPECT_THAT(::estd::make_slice(v), Slice(v.data(), 3U));
    }
}

TEST(Slice_dynamic, can_be_constructed_form_static_slice)
{
    int32_t data[10];
    ::estd::slice<int32_t, 10> s(data);

    ::estd::slice<int32_t> d(s);
    ASSERT_EQ(10U, d.size());
    ASSERT_EQ(data, d.data());
    d = s;
    ASSERT_EQ(10U, d.size());
    ASSERT_EQ(data, d.data());
}

TEST(Slice_dynamic, can_be_assigned_from_an_other_slice_of_same_type)
{
    ::estd::slice<int32_t> s;
    int32_t a[5] = {0};
    s            = ::estd::slice<int32_t>(a);
    EXPECT_THAT(s, Slice(a, 5U));
}

TEST(Slice_static, can_be_assigned_from_an_other_slice_of_same_type)
{
    int32_t a[5] = {1};
    ::estd::slice<int32_t, 5> s(a);
    int32_t b[5] = {0};
    s            = ::estd::make_static_slice(b);
    EXPECT_THAT(s, Slice(b, 5U));
}

} // namespace
