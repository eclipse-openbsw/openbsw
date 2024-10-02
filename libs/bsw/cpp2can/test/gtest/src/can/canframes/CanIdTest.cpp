// Copyright 2024 Accenture.

#include "can/canframes/CanId.h"

#include <gmock/gmock.h>

namespace
{
TEST(CanIdTest, test_static_functions)
{
    bool const extended  = true;
    bool const forceNoFd = true;

    ASSERT_EQ(0x7ffU, can::CanId::base(0x7ffU));

    ASSERT_EQ(0x800789abU, can::CanId::extended(0x789abU));

    ASSERT_EQ(0x800789abU, can::CanId::id(0x789abU, extended));
    ASSERT_EQ(0x000789abU, can::CanId::id(0x789abU, !extended));

    ASSERT_EQ(0x7ffU, can::CanId::id(0x7ff, !extended));
    ASSERT_EQ(0x800007ffU, can::CanId::id(0x7ff, extended));

    ASSERT_EQ(0x200007ffU, can::CanId::id(0x7ff, !extended, forceNoFd));
    ASSERT_EQ(0xA00007ffU, can::CanId::id(0x7ff, extended, forceNoFd));
    ASSERT_EQ(0x000007ffU, can::CanId::id(0x7ff, !extended, !forceNoFd));

    ASSERT_EQ(0x200007ffU, can::CanId::forceNoFd(0x7ff));

    ASSERT_EQ(0x7ffU, can::CanId::rawId(0xA00007ffU));
}

TEST(CanIdTest, test_type_based_value_retrival)
{
    ASSERT_EQ(0x7ffU, can::CanId::Base<0x7ffU>::value);
    ASSERT_EQ(0x800789abU, can::CanId::Extended<0x789abU>::value);
    ASSERT_EQ(0x800789abU, (can::CanId::Id<0x789abU, true>::value));
    ASSERT_EQ(0x7ffU, (can::CanId::Id<0x7ff, false>::value));
    uint32_t invalidId = can::CanId::Invalid::value;
    ASSERT_EQ(0xffffffffU, invalidId);
}

TEST(CanIdTest, test_checks)
{
    ASSERT_TRUE(can::CanId::isForceNoFd(0x200007ffU));
    ASSERT_FALSE(can::CanId::isForceNoFd(0x000007ffU));
}

TEST(CanIdTest, test_value_and_checks)
{
    ASSERT_EQ(0x78bU, can::CanId::rawId(can::CanId::Extended<0x78bU>::value));
    ASSERT_EQ(0x78bU, can::CanId::rawId(can::CanId::Base<0x78bU>::value));
    ASSERT_TRUE(can::CanId::isValid(can::CanId::Base<0x07ffU>::value));
    ASSERT_FALSE(can::CanId::isValid(can::CanId::Base<0x7ffU>::value | 0x40000000U));
    ASSERT_TRUE(can::CanId::isValid(can::CanId::Extended<0x1fffffffU>::value));
    ASSERT_FALSE(can::CanId::isValid(can::CanId::Extended<0x1fffffffU>::value | 0x40000000U));
    ASSERT_FALSE(can::CanId::isValid(can::CanId::Invalid::value));
    ASSERT_TRUE(can::CanId::isExtended(can::CanId::Extended<0x789U>::value));
    ASSERT_FALSE(can::CanId::isBase(can::CanId::Extended<0x789U>::value));
    ASSERT_FALSE(can::CanId::isExtended(can::CanId::Base<0x789U>::value));
    ASSERT_TRUE(can::CanId::isBase(can::CanId::Base<0x789U>::value));
}

} // namespace
