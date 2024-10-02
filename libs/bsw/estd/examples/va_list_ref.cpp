// Copyright 2024 Accenture.

#include <estd/va_list_ref.h>

#include <gtest/gtest.h>

TEST(VaListRefExample, variable)
{
    // [EXAMPLE_VA_LIST_REF_VARIABLE_START]
    uint32_t t = 17U;
    estd::internal::_va_list_ref<uint32_t> cut(t);
    EXPECT_EQ(17U, cut.get());
    estd::internal::_va_list_ref<uint32_t> copy(cut);
    EXPECT_EQ(17U, copy.get());
    // [EXAMPLE_VA_LIST_REF_VARIABLE_END]
}

TEST(VaListRefExample, array)
{
    // [EXAMPLE_VA_LIST_REF_ARRAY_START]
    using ArrayType = uint32_t[2];
    uint32_t t[2]   = {17U, 18U};
    estd::internal::_va_list_ref<ArrayType> cut(t);
    {
        ArrayType& value = cut.get();
        EXPECT_EQ(17U, value[0]);
        EXPECT_EQ(18U, value[1]);
    }
    estd::internal::_va_list_ref<ArrayType> copy(cut);
    {
        ArrayType& value = copy.get();
        EXPECT_EQ(17U, value[0]);
        EXPECT_EQ(18U, value[1]);
    }
    // [EXAMPLE_VA_LIST_REF_ARRAY_END]
}

// [EXAMPLE_VA_LIST_REF_CONSTRUCTOR_START]
static char const* cptr = "a";

void testCopyConstructor(int start, ...)
{
    va_list ap;
    va_start(ap, start);
    estd::va_list_ref ref(ap);
    estd::va_list_ref cut(ref);
    ASSERT_EQ(cptr, va_arg(cut.get(), char const*));
    ASSERT_EQ(2U, va_arg(cut.get(), uint32_t));
    ASSERT_EQ(45345ULL, va_arg(cut.get(), uint64_t));
    va_end(ap);
}

// [EXAMPLE_VA_LIST_REF_CONSTRUCTOR_END]

TEST(VaListRefExample, stdva_list)
{
    // [EXAMPLE_VA_LIST_REF_CONSTRUCTOR_CALL_START]
    testCopyConstructor(1, cptr, 2, 45345ULL);
    // [EXAMPLE_VA_LIST_REF_CONSTRUCTOR_CALL_END]
}
