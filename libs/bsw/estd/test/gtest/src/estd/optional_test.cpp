// Copyright 2024 Accenture.

#include "estd/optional.h"

#include "estd/assert.h"
#include "estd/uncopyable.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(optional, is_invalid_by_default)
{
    ::estd::optional<int32_t> i;
    EXPECT_FALSE(i.has_value());
}

TEST(optional, two_invalid_of_the_same_type_optionals_are_equal)
{
    ::estd::optional<int32_t> i0 = 7;
    ::estd::optional<int32_t> i1 = 5;

    EXPECT_TRUE(i0 == i0);
    EXPECT_TRUE(i1 == i1);
    EXPECT_TRUE(i0 != i1);
    EXPECT_FALSE(i0 == i1);

    i0.reset();
    EXPECT_TRUE(i0 == i0);
    EXPECT_TRUE(i1 == i1);
    EXPECT_TRUE(i0 != i1);
    EXPECT_FALSE(i1 == i0);
    EXPECT_FALSE(i0 == i1);

    i1.reset();
    EXPECT_TRUE(i0 == i0);
    EXPECT_TRUE(i1 == i1);
    EXPECT_TRUE(i0 == i1);
    EXPECT_FALSE(i0 != i1);
}

class SomeClass
{
public:
    int i;

    SomeClass(int i) : i(i) {}
};

TEST(optional, can_be_assigned_a_new_value)
{
    ::estd::optional<int32_t> i;
    EXPECT_FALSE(i.has_value());

    i = 10;
    EXPECT_TRUE(i.has_value());

    ::estd::optional<int32_t>* tmp = &i;
    *tmp                           = i;
    EXPECT_TRUE(i.has_value());

    EXPECT_EQ(10, *i);

    i = ::estd::none;
    EXPECT_FALSE(i.has_value());

    ::estd::optional<SomeClass> some_class;
    EXPECT_FALSE(some_class.has_value());
    some_class = 6;
    EXPECT_TRUE(some_class.has_value());
}

TEST(optional, can_be_copied_from_and_assigned_from_an_non_empty_other_optional)
{
    ::estd::optional<int32_t> a = 11;
    ::estd::optional<int32_t> b(a);

    EXPECT_TRUE(a.has_value());
    EXPECT_TRUE(b.has_value());

    a = 10;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(10, *a);

    a = b;

    EXPECT_EQ(11, *a);
}

TEST(optional, can_be_copied_from_and_assigned_from_an_empty_other_optional)
{
    ::estd::optional<int32_t> a;
    ::estd::optional<int32_t> b(a);

    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(b.has_value());

    a = 10;
    EXPECT_TRUE(a.has_value());

    a = b;

    EXPECT_FALSE(b.has_value());
}

TEST(optional, can_be_constructed_with_a_value)
{
    ::estd::optional<int32_t> i(5);
    ASSERT_TRUE(i.has_value());
    EXPECT_EQ(5, *i);
}

TEST(optional, a_new_object_can_be_inserted_with_placement_new)
{
    ::estd::optional<int32_t> i;
    EXPECT_FALSE(i.has_value());

    i.emplace().construct(6);
    ASSERT_TRUE(i.has_value());

    EXPECT_EQ(6, *i);
}

TEST(optional, can_be_defaulted_to_a_value)
{
    ::estd::optional<int32_t> i;

    EXPECT_EQ(5, i.value_or(5));

    i = 6;
    EXPECT_EQ(6, i.value_or(5));
}

TEST(optional, optional_values_have_distinct_memory_locations)
{
    ::estd::optional<int32_t> i = 1;
    ::estd::optional<int32_t> j = 2;

    EXPECT_EQ(1, *i);
    EXPECT_EQ(2, *j);

    i = j;

    EXPECT_EQ(2, *i);
    EXPECT_EQ(2, *j);

    i = 3;
    j = 4;

    EXPECT_EQ(3, *i);
    EXPECT_EQ(4, *j);
}

struct Dummy
{
    uint32_t magic; // This is used to distinguish initialized vs non-initialized memory
    static int32_t constructions;
    static int32_t destructions;
    static int32_t copies;

    Dummy() : magic(12345) { constructions++; }

    Dummy(Dummy const& orig)
    {
        estd_assert(orig.magic == 12345);
        magic = 12345;
        copies++;
    }

    Dummy& operator=(Dummy const& rhs)
    {
        estd_assert(magic == 12345);
        estd_assert(rhs.magic == 12345);
        copies++;
        return *this;
    }

    ~Dummy()
    {
        magic = 0;
        destructions++;
    }
};

int32_t Dummy::constructions = 0;
int32_t Dummy::destructions  = 0;
int32_t Dummy::copies        = 0;

TEST(optional, respects_alignment_of_T)
{
    EXPECT_EQ(alignof(uint8_t), alignof(::estd::optional<uint8_t>));
    EXPECT_EQ(0U, alignof(::estd::optional<uint32_t>) % alignof(uint32_t));
    EXPECT_EQ(0U, alignof(::estd::optional<uint64_t>) % alignof(uint64_t));
}

// clang warns about self assignment, which is good and we want to keep, but we also want to keep
// (and test!) safe handling of self assignment, in the case that someone does it anyway.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
TEST(optional, is_safe_to_self_assign)
{
    ::estd::optional<Dummy> i;
    EXPECT_FALSE(i.has_value());
    i = i;
    EXPECT_FALSE(i.has_value());
    i.emplace().construct();
    EXPECT_TRUE(i.has_value());
    i = i;
    EXPECT_TRUE(i.has_value());
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

TEST(optional, calls_the_destructor_in_emplace_and_reset)
{
    Dummy::constructions = 0;
    Dummy::destructions  = 0;
    Dummy::copies        = 0;

    ::estd::optional<Dummy> i;

    EXPECT_EQ(0, Dummy::constructions);
    EXPECT_EQ(0, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);

    i.emplace().construct();

    EXPECT_EQ(1, Dummy::constructions);
    EXPECT_EQ(0, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);

    i.emplace().construct();

    EXPECT_EQ(2, Dummy::constructions);
    EXPECT_EQ(1, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);

    i.reset();

    EXPECT_EQ(2, Dummy::constructions);
    EXPECT_EQ(2, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);
}

TEST(optional, calls_the_destructor_on_destruction)
{
    Dummy::constructions = 0;
    Dummy::destructions  = 0;
    Dummy::copies        = 0;

    {
        ::estd::optional<Dummy> i;

        EXPECT_EQ(0, Dummy::constructions);
        EXPECT_EQ(0, Dummy::destructions);
        EXPECT_EQ(0, Dummy::copies);

        i.emplace().construct();

        EXPECT_EQ(1, Dummy::constructions);
        EXPECT_EQ(0, Dummy::destructions);
        EXPECT_EQ(0, Dummy::copies);
    }

    EXPECT_EQ(1, Dummy::constructions);
    EXPECT_EQ(1, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);
}

::estd::optional<int32_t> maybe_int() { return 5; }

TEST(optional, can_be_returned_from_a_function)
{
    ::estd::optional<int32_t> i = maybe_int();
    ASSERT_TRUE(i.has_value());
    EXPECT_EQ(5, *i);
}

TEST(Optional_Ref, defaults_to_invalid)
{
    ::estd::optional<int32_t&> ir;
    EXPECT_FALSE(ir.has_value());
}

TEST(optional, value_can_be_accessed_by_arrow_operator)
{
    ::estd::optional<SomeClass> optionalObject = SomeClass(42);
    EXPECT_EQ(optionalObject->i, 42);
    EXPECT_EQ(&(optionalObject->i), &(optionalObject.get().i));
}

TEST(optional, arrow_operator_asserts_that_value_exists)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::optional<SomeClass> emptyOptional;
    ASSERT_THROW({ (void)emptyOptional->i; }, ::estd::assert_exception);
}

TEST(Optional_Ref, asserts_on_invalid_dereference)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::optional<int32_t&> ir;
    ASSERT_THROW({ *ir; }, ::estd::assert_exception);
}

TEST(Optional_Ref, acts_like_a_pointer)
{
    int32_t i = 0;
    ::estd::optional<int32_t&> ir(i);
    ASSERT_TRUE(ir.has_value());

    EXPECT_EQ(i, *ir);
    ++i;
    EXPECT_EQ(i, *ir);
    ++*ir;
    EXPECT_EQ(i, *ir);

    int32_t j = 9;
    ir.reset(j);

    EXPECT_EQ(j, *ir);
    ++j;
    EXPECT_EQ(j, *ir);
    ++*ir;
    EXPECT_EQ(j, *ir);

    ir.reset();
    EXPECT_FALSE(ir.has_value());

    ir = i;
    EXPECT_TRUE(ir.has_value());

    ir = ::estd::none;
    EXPECT_FALSE(ir.has_value());
}

TEST(Optional_Ref, is_always_copyable)
{
    ::estd::optional<::estd::uncopyable&> r;
    ::estd::optional<::estd::uncopyable&> r2(r);
    EXPECT_FALSE(r2.has_value());
}

TEST(optional, check_implicit_bool_conversion)
{
    ::estd::optional<int> obj = 5;
    EXPECT_TRUE(obj);
    ::estd::optional<uint64_t> empty;
    EXPECT_FALSE(empty);
}

} // namespace
