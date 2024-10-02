// Copyright 2024 Accenture.

#include "estd/vec.h"

#include "estd/slice.h"

#include <gtest/gtest.h>

#include <cassert>

namespace
{
void example_construction()
{
    // EXAMPLE_START construction
    // default constructed - size() = 0, max_size = 8, free() = 8
    ::estd::vec<int, 8> int_vec;

    // construction of vec prefilled with a given number of elements
    ::estd::vec<uint8_t, 10> byte_vec1(8);      // 8 elements of value 0
    ::estd::vec<uint8_t, 10> byte_vec2(8, 255); // 8 elements of value 255
    // EXAMPLE_END construction
}

void example_iterate()
{
    // EXAMPLE_START iterate

    ::estd::vec<uint8_t, 10> vec5(8);

    // iterate over all elements, modify them
    uint8_t counter = 0;
    for (auto& x : vec5)
    {
        x = counter++;
    }

    // iterate over elements in reverse order
    for (auto rev_it = vec5.rbegin(); rev_it != vec5.rend(); ++rev_it)
    {
        *rev_it = (*rev_it) * (*rev_it);
    }

    // EXAMPLE_END iterate
}

#if 0
void example_element_access() // don't run - asserts on purpose!
{
    // EXAMPLE_START element_access
    ::estd::vec<uint8_t, 10> v;
    v.push_back(1);
    v.push_back(2);
    EXPECT_EQ(2, v.size());
    EXPECT_EQ(8, v.free());

    v.at(0) = 42; // assign 42 to the first element
    v[1]    = 43; // assign 43 to the second element

    // access out of bounds - will fail an assert
    v.at(2);
    // access out of bounds - undefined behavior (use operator[] with caution!)
    v[2];

    uint8_t* ptr = v.data(); // points to first element
    EXPECT_EQ(ptr, &v.front());
    // EXAMPLE_END element_access
}
#endif

#if 0
void example_modify() // don't run - asserts on purpose!
{
    // EXAMPLE_START modify
    ::estd::vec<int32_t, 10> vec_int;

    // resize vec from size 0 to size 7, default constructing elements (value 0 for integers)
    vec_int.resize(7);

    // assign range to vec
    uint8_t const testData[] = {0, 1, 2, 3, 4};
    vec_int.assign(std::begin(testData), std::end(testData));

    // pushing beyond the limit will result in a failed assert.
    ::estd::vec<uint8_t, 1> vec_1;
    vec_1.push_back(0);
    vec_1.push_back(1); // assert fails!

    class Foo
    {
    public:
        Foo(int32_t m, int32_t n) : _m(m), _n(n) {}

        Foo(Foo const&)            = delete;
        void operator=(Foo const&) = delete;

        int32_t _m;
        int32_t _n;
    };

    // instantiation of a vec of a noncopyable non default constructible type
    ::estd::vec<Foo, 10> vec_foo;
    vec_foo.emplace_back(13, 17);
    // EXAMPLE_END modify
}
#endif

} // namespace

TEST(Vec, run_examples)
{
    example_construction();
    example_iterate();
}
