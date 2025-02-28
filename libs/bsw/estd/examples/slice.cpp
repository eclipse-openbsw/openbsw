// Copyright 2024 Accenture.

#include "estd/slice.h"

#include "estd/array.h"
#include "estd/assert.h"
#include "estd/vector.h"

#include <gtest/gtest.h>

#pragma GCC diagnostic ignored "-Wunused-variable"

// clang-format off
namespace
{
void example_construction()
{
    {
    // [EXAMPLE_BEGIN estd static construction from C array]
    uint8_t data[] = {0, 1, 2, 3, 4};

    // statically-sized slice of size 5
    ::estd::slice<uint8_t, 5U> slice5(data);

    // statically-sized slice of size 3 which points to the first three elements
    ::estd::slice<uint8_t, 3U> slice3(data);
    // [EXAMPLE_END estd static construction from C array]
    }

    {
    // [EXAMPLE_BEGIN estd dynamic construction from C array]
    uint8_t data[] = {0, 1, 2, 3, 4};

    // dynamically-sized slice of size 5
    ::estd::slice<uint8_t> dynSizeSlice(data);

    // dynamically-sized slice can be constructed from a statically sized slice:
    ::estd::slice<uint8_t, 5U> slice5(data);

    // [EXAMPLE_END estd dynamic construction from C array]
    }

    {
    // [EXAMPLE_BEGIN estd construction from an estd array]
    ::estd::array<int32_t, 4U> data = {0, 1, 2, 3};
    ::estd::slice<int32_t, 4U> slice4(data);
    // EXAMPLE_END estd construction from an estd array
    }

    {
    // [EXAMPLE_BEGIN estd construction from pointer]
    uint8_t data[]                       = {0, 1, 2, 3, 4};
    uint8_t* pointer                     = data;
    ::estd::slice<uint8_t, 5U> slice5_fp = ::estd::slice<uint8_t, 5U>::from_pointer(pointer);
    // [EXAMPLE_END estd construction from pointer]
    }

    {
    // [EXAMPLE_BEGIN estd dynamic construction from estd vector]
    ::estd::declare::vector<int32_t, 10> v1;
    ::estd::slice<int32_t> s1(v1);

    ::estd::declare::vector<int32_t, 20> v2;
    ::estd::vector<int32_t>& v3 = v2;
    ::estd::slice<int32_t> s3(v3);
    // [EXAMPLE_END estd dynamic construction from estd vector]
    }

#if 0 // this won't compile, therefore disabled
    // [EXAMPLE_BEGIN estd array too small]
    ::estd::slice<uint8_t, 6U> slice6(data); // won't compile - array is smaller than the size of slice
    // [EXAMPLE_END estd array too small]
#endif
}

void example_make_slice()
{
    // [EXAMPLE_BEGIN estd make_slice]

    uint32_t data[] = {0, 1, 2, 3, 4};
    auto staticSlice = ::estd::make_static_slice(data); // staticSlice has type slice<uint32_t, 5>

    // [EXAMPLE_END estd make_slice]
}

void example_iterate()
{
    // [EXAMPLE_BEGIN estd iterate]
    uint32_t data[] = {0, 1, 2, 3, 4};
    ::estd::slice<uint32_t> slice5{data};

    // iterate over all elements, incrementing each of them:
    for (auto& i : slice5)
    {
        ++i;
    }

    // iterate over elements in reverse order
    for (auto rev_it = slice5.rbegin(); rev_it != slice5.rend(); ++rev_it)
    {
        *rev_it = *rev_it + 10;
    }

    // [EXAMPLE_END estd iterate]
}

void example_access_at()
{
    // [EXAMPLE_BEGIN estd access_at]
    uint8_t data[] = {0, 1, 2, 3, 4};
    ::estd::slice<uint8_t, 5> const slice5(data);

    slice5.at<0>() = 42; // assign 42 to the first element

    // [EXAMPLE_END estd access_at]
}

void example_subviews()
{
    // [EXAMPLE_BEGIN estd subviews]
    uint8_t data[] = {0, 1, 2, 3, 4};
    ::estd::slice<uint8_t> const slice(data);

    // [EXAMPLE_END estd subviews]
}

void example_subviews_static()
{
    // [EXAMPLE_BEGIN estd subviews_static]
    uint8_t data[] = {0, 1, 2, 3, 4};
    ::estd::slice<uint8_t, 5U> const slice5(data);
    ::estd::slice<uint8_t, 4U> slice4 = slice5.offset<1>();   // skip 1 element: {1, 2, 3, 4}
    ::estd::slice<uint8_t, 3U> slice3 = slice5.subslice<3>(); // first 3 elements: {0, 1, 2}
    // [EXAMPLE_END estd subviews_static]
}

void example_modify()
{
    // [EXAMPLE_BEGIN estd modify]
    uint8_t data[] = {0, 1, 2, 3, 4};
    ::estd::slice<uint8_t> slice(data);

    slice.advance(1U); // slice's pointer incremented by one, now points to {1, 2, 3, 4}
    slice.trim(2U);    // slice's size decreased to 2, now points to {1, 2}
    // [EXAMPLE_END estd modify]
}

} // namespace

TEST(Slice, run_examples)
{
    example_construction();
    example_make_slice();
    example_iterate();
    example_subviews();
    example_subviews_static();
    example_modify();
    example_access_at();
}

// clang-format off
