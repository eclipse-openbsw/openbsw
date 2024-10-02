// Copyright 2024 Accenture.

// Include class we are testing first to check for include dependencies
#include "estd/iterator.h"

#include "estd/vector.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(Iterator, NextWithoutParamAdvancesByOne)
{
    ::estd::declare::vector<uint32_t, 10> v;
    v.push_back(0U);
    v.push_back(1U);
    v.push_back(2U);
    v.push_back(3U);
    ::estd::vector<uint32_t>::iterator itr = v.begin();
    ASSERT_EQ(0U, *itr);
    ASSERT_EQ(1U, *::estd::next(itr));
}

TEST(Iterator, NextWithoutParamAdvancesByOneAndLeavesInputIteratorUntouched)
{
    ::estd::declare::vector<uint32_t, 10> v;
    v.push_back(0U);
    v.push_back(1U);
    v.push_back(2U);
    v.push_back(3U);
    ::estd::vector<uint32_t>::iterator itr = v.begin();
    ASSERT_EQ(0U, *itr);
    ASSERT_EQ(1U, *::estd::next(itr));
    ASSERT_EQ(0U, *itr);
}

TEST(Iterator, NextWithParamAdvancesByN)
{
    ::estd::declare::vector<uint32_t, 10> v;
    v.push_back(0U);
    v.push_back(1U);
    v.push_back(2U);
    v.push_back(3U);
    ::estd::vector<uint32_t>::iterator itr = v.begin();
    ASSERT_EQ(0U, *itr);
    ASSERT_EQ(1U, *::estd::next(itr, 1));
    ASSERT_EQ(2U, *::estd::next(itr, 2));
    ASSERT_EQ(3U, *::estd::next(itr, 3));
}

} // namespace
