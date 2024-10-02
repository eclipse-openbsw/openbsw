// Copyright 2024 Accenture.

#include "estd/vector.h"

#include <gtest/gtest.h>

namespace
{
void example_construction()
{
    // EXAMPLE_START construction
    // construction of a defaulted ::estd::declare::vector that stores up to 10 elements
    ::estd::declare::vector<uint8_t, 10> vec1;

    // construction of vectors prefilled with a given number of elements
    ::estd::declare::vector<uint8_t, 10> vec2(8);
    ::estd::declare::vector<uint8_t, 10> vec3(8, 255);

    // construction of a vector from another vector
    ::estd::declare::vector<uint8_t, 20> vec4(vec3);

    // EXAMPLE_END construction
}

void example_iterate()
{
    // EXAMPLE_START iterate

    ::estd::declare::vector<int, 10> int_vec(8);

    // iterate over all elements, modify them
    uint8_t counter = 0;
    for (auto& x : int_vec)
    {
        x = counter++;
    }

    // iterate over elements in reverse order
    for (auto rev_it = int_vec.rbegin(); rev_it != int_vec.rend(); ++rev_it)
    {
        *rev_it = (*rev_it) * (*rev_it);
    }

    // EXAMPLE_END iterate
}

#if 0
void example_element_access() // don't run - asserts on purpose!
{
    // EXAMPLE_START element_access
    ::estd::declare::vector<uint8_t, 10> vec;
    vec.push_back(1);
    vec.push_back(2);

    EXPECT_EQ(2, vec.size());
    ASSERT_FALSE(vec.empty());
    ASSERT_FALSE(vec.full());
    EXPECT_EQ(2, vec.back());

    vec.at(0) = 42; // assign 42 to the first element
    vec[1]    = 43; // assign 43 to the second element

    // access out of bounds - will fail an assert
    vec.at(2);
    // access out of bounds - undefined behavior (use operator[] with caution!)
    vec[2];

    uint8_t* ptr = vec.data(); // points to first element
    EXPECT_EQ(ptr, &vec.front());
    // EXAMPLE_END element_access
}
#endif

#if 0
void example_modify() // don't run - asserts on purpose!
{
    // EXAMPLE_START modify
    ::estd::declare::vector<uint8_t, 10> vec7;

    // resize vector from size 0 to size 2, growing two elements with value 255
    vec7.resize(2, 255);

    ::estd::declare::vector<uint8_t, 5> vec8;

    // assign range to vector
    uint8_t const testData[] = {0, 1, 2, 3, 4};
    vec8.assign(std::begin(testData), std::end(testData));

    // pushing beyond the limit will result in a failed assert.
    ::estd::declare::vector<uint8_t, 1> vec9;
    vec9.push_back(0);
    vec9.push_back(1); // assert fails!

    class Foo
    {
    public:
        Foo(int32_t m, int32_t n) : _m(m), _n(n) {}

        Foo(Foo const&)            = delete;
        void operator=(Foo const&) = delete;

        int32_t _m;
        int32_t _n;
    };

    // instantiation of a vector of a noncopyable non default constructible type
    ::estd::declare::vector<Foo, 10> vec10;
    vec10.emplace_back().construct(13, 17);

    ::estd::declare::vector<uint8_t, 10> vec11;
    vec11.clear();

    // insert elements in the vector
    vec11.insert(vec11.begin(), 1);
    vec11.insert(vec11.begin() + 1, 2, 1);
    vec11.emplace(vec11.begin() + 1U).construct(11);

    // remove elements in the vector
    vec11.pop_back();
    vec11.erase(vec11.begin());
    vec11.erase(vec11.end() - 2, vec11.end());

    // assign elements to the vector
    vec11.assign(10, 42);
    vec11.assign(vec11.begin(), vec11.end());
    // EXAMPLE_END modify
}
#endif

void example_compare()
{
    // EXAMPLE_START compare
    uint8_t testData[] = {0, 1, 2, 3, 4};
    ::estd::declare::vector<int32_t, 5> vec_0_1_2_3_4;
    vec_0_1_2_3_4.assign(std::begin(testData), std::end(testData));

    ::estd::declare::vector<int32_t, 3> vec_99_99(2, 99);
    ::estd::declare::vector<int32_t, 3> vec_99_15(2);
    vec_99_15[0] = 99;
    vec_99_15[1] = 15;

    // vectors of different sizes are not equal and "less than/greater than" relation is undefined
    EXPECT_TRUE(vec_99_99 != vec_0_1_2_3_4);
    EXPECT_FALSE(vec_99_99 > vec_0_1_2_3_4);
    EXPECT_FALSE(vec_99_99 < vec_0_1_2_3_4);
    EXPECT_FALSE(vec_0_1_2_3_4 > vec_99_99);
    EXPECT_FALSE(vec_0_1_2_3_4 < vec_99_99);

    // vectors of same size are compared lexicographically
    EXPECT_TRUE(vec_99_99 != vec_99_15);
    EXPECT_TRUE(vec_99_99 > vec_99_15);
    EXPECT_FALSE(vec_99_99 < vec_99_15);

    // vectors with same element size and value
    EXPECT_TRUE(vec_0_1_2_3_4 == vec_0_1_2_3_4);
    EXPECT_TRUE(vec_0_1_2_3_4 <= vec_0_1_2_3_4);
    EXPECT_TRUE(vec_0_1_2_3_4 >= vec_0_1_2_3_4);
    // EXAMPLE_END compare
}

} // namespace

TEST(Vector, run_examples)
{
    example_construction();
    example_iterate();
    example_compare();
}
