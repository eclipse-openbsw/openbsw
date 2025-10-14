// Copyright 2024 Accenture.

#include "util/types/Enum.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::util::types;

struct TestEnum
{
    enum type
    {
        WHITE,
        GREEN,
        BLUE,
        RED,
        YELLOW,
        BLACK
    };
};

struct ElevatorLevel
{
    enum type
    {
        UG,
        EG,
        FIRST_FLOOR,
        SECOND_FLOOR
    };
};

TEST(Enum, default_constructor_initializes_with_first_enum_element)
{
    Enum<TestEnum> color;
    ASSERT_THAT(color.value(), ::testing::Eq(TestEnum::WHITE));
}

TEST(Enum, fromUnderlying_creates_an_enum_based_on_an_enum_of_the_underlying_type)
{
    Enum<TestEnum> color = Enum<TestEnum>::fromUnderlying(TestEnum::GREEN);
    ASSERT_THAT(color.value(), ::testing::Eq(TestEnum::GREEN));
}

TEST(Enum, enum_gets_initialized_by_other_enum)
{
    Enum<TestEnum> color1(TestEnum::BLUE);
    Enum<TestEnum> color2(color1);
    ASSERT_THAT(color1.value(), ::testing::Eq(color2.value()));
}

TEST(Enum, value_type_constructor_initializes_to_specified_value)
{
    Enum<TestEnum> color(TestEnum::BLUE);
    ASSERT_THAT(color.value(), ::testing::Eq(TestEnum::BLUE));
}

TEST(Enum, supports_assignment)
{
    Enum<TestEnum> rhs(TestEnum::RED);
    Enum<TestEnum> color;
    ASSERT_THAT(color.value(), ::testing::Ne(TestEnum::RED));
    color = rhs;
    ASSERT_THAT(color.value(), ::testing::Eq(TestEnum::RED));
}

TEST(Enum, equals_operator_returns_true_for_two_equal_enums)
{
    Enum<TestEnum> lhs;
    Enum<TestEnum> rhs;
    ASSERT_THAT(lhs == rhs, ::testing::Eq(true));
}

TEST(Enum, equals_operator_returns_false_for_two_unequal_enums)
{
    Enum<TestEnum> lhs(TestEnum::BLUE);
    Enum<TestEnum> rhs(TestEnum::WHITE);
    ASSERT_THAT(lhs == rhs, ::testing::Eq(false));
}

TEST(Enum, unequals_operator_returns_false_for_two_equal_enums)
{
    Enum<TestEnum> lhs;
    Enum<TestEnum> rhs;
    ASSERT_THAT(lhs != rhs, ::testing::Eq(false));
}

TEST(Enum, equals_operator_returns_true_for_two_unequal_enums)
{
    Enum<TestEnum> lhs(TestEnum::BLUE);
    Enum<TestEnum> rhs(TestEnum::WHITE);
    ASSERT_THAT(lhs != rhs, ::testing::Eq(true));
}

TEST(Enum, lesser_than_operator_returns_false_if_lhs_is_bigger_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::SECOND_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs < rhs, ::testing::Eq(false));
}

TEST(Enum, lesser_than_operator_returns_true_if_lhs_is_smaller_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::UG);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs < rhs, ::testing::Eq(true));
}

TEST(Enum, lesser_than_or_operator_returns_false_if_lhs_is_bigger_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::SECOND_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs <= rhs, ::testing::Eq(false));
}

TEST(Enum, lesser_than_or_operator_returns_true_if_lhs_is_equal_to_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::FIRST_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs <= rhs, ::testing::Eq(true));
}

TEST(Enum, lesser_than_or_operator_returns_true_if_lhs_is_lesser_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::FIRST_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::SECOND_FLOOR);
    ASSERT_THAT(lhs <= rhs, ::testing::Eq(true));
}

TEST(Enum, greater_than_operator_returns_true_if_lhs_is_bigger_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::SECOND_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs > rhs, ::testing::Eq(true));
}

TEST(Enum, greater_than_operator_returns_false_if_lhs_is_smaller_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::UG);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs > rhs, ::testing::Eq(false));
}

TEST(Enum, greater_than_or_operator_returns_true_if_lhs_is_bigger_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::SECOND_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs >= rhs, ::testing::Eq(true));
}

TEST(Enum, greater_than_or_operator_returns_true_if_lhs_is_equal_to_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::FIRST_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::FIRST_FLOOR);
    ASSERT_THAT(lhs >= rhs, ::testing::Eq(true));
}

TEST(Enum, greater_than_or_operator_returns_false_if_lhs_is_lesser_than_rhs)
{
    Enum<ElevatorLevel> lhs(ElevatorLevel::FIRST_FLOOR);
    Enum<ElevatorLevel> rhs(ElevatorLevel::SECOND_FLOOR);
    ASSERT_THAT(lhs >= rhs, ::testing::Eq(false));
}

} // namespace
