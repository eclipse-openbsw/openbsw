// Copyright 2024 Accenture.

#include "estd/index_sequence.h"

#include <gmock/gmock.h>

using namespace ::testing;

template<typename T>
struct UnpackSequence;

template<size_t... Index>
struct UnpackSequence<::estd::index_sequence<Index...>>
{
    size_t values[sizeof...(Index)]{Index...};
};

TEST(index_sequence, index_sequence)
{
    EXPECT_THAT(UnpackSequence<::estd::make_index_sequence<1>>().values, ElementsAre(0));
    EXPECT_THAT(UnpackSequence<::estd::make_index_sequence<3>>().values, ElementsAre(0, 1, 2));
    EXPECT_THAT(
        UnpackSequence<::estd::make_index_sequence<5>>().values, ElementsAre(0, 1, 2, 3, 4));
}
