// Copyright 2024 Accenture.

#include "estd/variant.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

namespace
{
struct MyStruct
{};

struct DoNothing
{
    template<typename T>
    void operator()(T const&)
    {}
};
} // namespace

TEST(variant_T_do, void_tests)
{
    using tl = ::estd::make_type_list<void>::type;

    // invalid index
    EXPECT_FALSE(::estd::variant_T_do<tl>::compare(6, nullptr, nullptr));

    uint8_t* lhsMem = nullptr;
    uint8_t* rhsMem = nullptr;
    DoNothing emptyVisitor;

    // do nothing functions
    ::estd::variant_T_do<tl>::assign<false>(1, lhsMem, rhsMem);
    ::estd::variant_T_do<tl>::copy(2, lhsMem, rhsMem);
    ::estd::variant_T_do<tl>::destroy(3, lhsMem);
    ::estd::variant_T_do<tl>::call<DoNothing, void>(1, lhsMem, emptyVisitor);
}

TEST(variant, index_of)
{
    using V2 = ::estd::variant<int, bool>;

    EXPECT_EQ(0U, V2::index_of<int>());
    EXPECT_EQ(1U, V2::index_of<bool>());

    using V5 = ::estd::variant<uint8_t, size_t, bool, MyStruct, int64_t>;

    EXPECT_EQ(0U, V5::index_of<uint8_t>());
    EXPECT_EQ(1U, V5::index_of<size_t>());
    EXPECT_EQ(2U, V5::index_of<bool>());
    EXPECT_EQ(3U, V5::index_of<MyStruct>());
    EXPECT_EQ(4U, V5::index_of<int64_t>());
}

// If a variant is default initialized and the first type is able to be default initialized, then
// the variant will be initialized to a default-initialized value of the first variant type.
TEST(variant, default_constructor_initializes_to_default_initialized_first_type)
{
    ::estd::variant<int, bool> i_or_b;

    EXPECT_TRUE(i_or_b.is<int>());
    EXPECT_FALSE(i_or_b.is<bool>());

    EXPECT_TRUE(::estd::holds_alternative<int>(i_or_b));
    EXPECT_FALSE(::estd::holds_alternative<bool>(i_or_b));

    EXPECT_EQ(0U, i_or_b.index());

    EXPECT_EQ(0, i_or_b.get<int>());
    EXPECT_EQ(0, ::estd::get<int>(i_or_b));
    EXPECT_EQ(i_or_b, int(0));
    EXPECT_EQ(i_or_b, 0);
    EXPECT_NE(i_or_b, false);

    // If the first type is _not_ default initializable, a compiler error will be thrown.
    // struct NonDefaultConstructibleType
    //{
    //     explicit NonDefaultConstructibleType(int);
    //     // OR
    //     NonDefaultConstructibleType() = delete;
    // };
    //::estd::variant<NonDefaultConstructibleType, int> willNotCompile;
}

TEST(variant, can_be_initialized_to_first)
{
    ::estd::variant<int, bool> i_or_b(int(7));

    EXPECT_TRUE(i_or_b.is<int>());
    EXPECT_FALSE(i_or_b.is<bool>());

    EXPECT_TRUE(::estd::holds_alternative<int>(i_or_b));
    EXPECT_FALSE(::estd::holds_alternative<bool>(i_or_b));

    EXPECT_EQ(0U, i_or_b.index());

    EXPECT_EQ(7, i_or_b.get<int>());
    EXPECT_EQ(7, ::estd::get<int>(i_or_b));
    EXPECT_EQ(i_or_b, int(7));
    EXPECT_EQ(i_or_b, 7);
    EXPECT_NE(i_or_b, false);
}

TEST(variant, can_be_initialized_to_second)
{
    ::estd::variant<int, bool> i_or_b(true);

    EXPECT_TRUE(i_or_b.is<bool>());
    EXPECT_FALSE(i_or_b.is<int>());

    EXPECT_EQ(1U, i_or_b.index());

    EXPECT_EQ(true, i_or_b.get<bool>());
    EXPECT_NE(i_or_b, 3);
    EXPECT_EQ(i_or_b, true);
}

TEST(variant, can_be_assigned_a_new_value)
{
    ::estd::variant<int, bool> i_or_b(int(0));

    EXPECT_TRUE(i_or_b.is<int>());

    i_or_b = false;
    EXPECT_TRUE(i_or_b.is<bool>());
}

TEST(variant, can_be_copied_from_and_assigned_from_an_other_variant)
{
    ::estd::variant<int, bool> a = true;

    ::estd::variant<int, bool> b(a);

    EXPECT_TRUE(a.is<bool>());
    EXPECT_TRUE(b.is<bool>());

    EXPECT_EQ(a, b);

    b = 0;
    a = b;

    EXPECT_TRUE(a.is<int>());
    EXPECT_TRUE(b.is<int>());

    a = 1;

    EXPECT_NE(a, b);
}

struct Dummy
{
    static int constructions;
    static int destructions;
    static int copies;
    static int assignments;

    int nr;

    Dummy() : nr(constructions + copies) { constructions++; }

    Dummy(Dummy const& o) : nr(o.nr) { copies++; }

    ~Dummy() { destructions++; }

    Dummy& operator=(Dummy const& rhs)
    {
        assignments++;
        nr = rhs.nr;
        return *this;
    }

    bool operator==(Dummy const& rhs) const { return nr == rhs.nr; }
};

int Dummy::constructions = 0;
int Dummy::destructions  = 0;
int Dummy::copies        = 0;
int Dummy::assignments   = 0;

TEST(variant, calls_the_correct_destructor)
{
    Dummy::constructions = 0;
    Dummy::destructions  = 0;
    Dummy::copies        = 0;
    Dummy::assignments   = 0;

    {
        ::estd::variant<int, Dummy> v = 5;

        EXPECT_TRUE(v.is<int>());

        EXPECT_EQ(0, Dummy::constructions);
        EXPECT_EQ(0, Dummy::destructions);
        EXPECT_EQ(0, Dummy::copies);
        EXPECT_EQ(0, Dummy::assignments);
    }

    EXPECT_EQ(0, Dummy::constructions);
    EXPECT_EQ(0, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);

    {
        ::estd::variant<int, Dummy> v((Dummy()));

        EXPECT_TRUE(v.is<Dummy>());

        EXPECT_EQ(1, Dummy::constructions);
        EXPECT_EQ(1, Dummy::destructions);
        EXPECT_EQ(1, Dummy::copies);
        EXPECT_EQ(0, Dummy::assignments);

        v = 5;

        EXPECT_EQ(1, Dummy::constructions);
        EXPECT_EQ(2, Dummy::destructions);
        EXPECT_EQ(1, Dummy::copies);
        EXPECT_EQ(0, Dummy::assignments);

        v = Dummy();
    }

    EXPECT_EQ(2, Dummy::constructions);
    EXPECT_EQ(4, Dummy::destructions);
    EXPECT_EQ(2, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);
}

TEST(variant, a_new_object_can_be_inserted_with_placement_new)
{
    Dummy::constructions = 0;
    Dummy::destructions  = 0;
    Dummy::copies        = 0;
    Dummy::assignments   = 0;

    ::estd::variant<int, Dummy> v(42);

    EXPECT_EQ(0, Dummy::constructions);
    EXPECT_EQ(0, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);

    v.emplace<Dummy>().construct();

    EXPECT_EQ(1, Dummy::constructions);
    EXPECT_EQ(0, Dummy::destructions);
    EXPECT_EQ(0, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);

    EXPECT_TRUE(v.is<Dummy>());
}

TEST(variant, can_be_copied)
{
    Dummy::constructions = 0;
    Dummy::destructions  = 0;
    Dummy::copies        = 0;
    Dummy::assignments   = 0;

    ::estd::variant<int, Dummy> a(14);

    ::estd::variant<int, Dummy> b = Dummy();

    EXPECT_EQ(1, Dummy::constructions);
    EXPECT_EQ(1, Dummy::destructions);
    EXPECT_EQ(1, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);

    EXPECT_TRUE(a.is<int>());
    EXPECT_TRUE(b.is<Dummy>());

    a = b;

    EXPECT_EQ(1, Dummy::constructions);
    EXPECT_EQ(1, Dummy::destructions);
    EXPECT_EQ(2, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);

    EXPECT_TRUE(a.is<Dummy>());
    EXPECT_TRUE(b.is<Dummy>());

    EXPECT_EQ(a, b);
}

TEST(variant, can_be_assigned)
{
    Dummy::constructions = 0;
    Dummy::destructions  = 0;
    Dummy::copies        = 0;
    Dummy::assignments   = 0;

    ::estd::variant<int, Dummy> a((Dummy()));
    ::estd::variant<int, Dummy> b((Dummy()));

    EXPECT_EQ(2, Dummy::constructions);
    EXPECT_EQ(2, Dummy::destructions);
    EXPECT_EQ(2, Dummy::copies);
    EXPECT_EQ(0, Dummy::assignments);

    a = Dummy();

    EXPECT_EQ(3, Dummy::constructions);
    EXPECT_EQ(3, Dummy::destructions);
    EXPECT_EQ(2, Dummy::copies);
    EXPECT_EQ(1, Dummy::assignments);

    a = b;

    EXPECT_EQ(3, Dummy::constructions);
    EXPECT_EQ(3, Dummy::destructions);
    EXPECT_EQ(2, Dummy::copies);
    EXPECT_EQ(2, Dummy::assignments);

    EXPECT_TRUE(a.is<Dummy>());
    EXPECT_TRUE(b.is<Dummy>());
}

// clang warns about self assignment, which is good and we want to keep, but we also want to keep
// (and test!) safe handling of self assignment, in the case that someone does it anyway.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
TEST(variant, can_be_self_assigned)
{
    ::estd::variant<int, bool> a = int(1234);

    a = a;

    EXPECT_EQ(1234, a.get<int>());
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

TEST(variant, get_fails_type)
{
    ::estd::variant<int, bool> a = int(1234);
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ASSERT_THROW((a.get<bool>()), ::estd::assert_exception);
}

TEST(variant, const_get)
{
    ::estd::variant<int, bool> a = int(1234);

    ::estd::variant<int, bool> const& ca = a;

    EXPECT_EQ(1234, ca.get<int>());
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ASSERT_THROW((ca.get<bool>()), ::estd::assert_exception);
}

struct OrElse
{
    mutable bool failed;

    OrElse() : failed(false) {}

    int operator()() const
    {
        failed = true;
        return 999;
    }
};

TEST(variant, value_or_else)
{
    ::estd::variant<int, double> a = int(1234);

    OrElse orElse;

    EXPECT_EQ(1234, ::estd::value_or_else<int>(a, orElse));
    EXPECT_FALSE(orElse.failed);

    EXPECT_EQ(999, ::estd::value_or_else<double>(a, orElse));
    EXPECT_TRUE(orElse.failed);
}

struct Visitor
{
    int& _out;

    Visitor(int& out) : _out(out) {}

    void operator()(int) const { _out = 7; }

    void operator()(bool) const { _out = 8; }
};

struct VisitorR
{
    int operator()(int) const { return 7; }

    int operator()(bool) const { return 8; }
};

struct VisitorNonConst
{
    int _out = 0U;

    void operator()(int) { _out = 7; }

    void operator()(bool) { _out = 8; }
};

TEST(variant, visit)
{
    ::estd::variant<int, bool> a = int(1234);

    int out = 0;
    ::estd::visit<void>(Visitor(out), a);
    EXPECT_EQ(7, out);

    VisitorNonConst v;
    ::estd::visit<void>(v, a);
    EXPECT_EQ(7, v._out);

    EXPECT_EQ(7, ::estd::visit<int>(VisitorR(), a));

    a = false;
    EXPECT_EQ(8, ::estd::visit<int>(VisitorR(), a));
}

TEST(variant, less_than)
{
    ::estd::variant<int, bool> a = int(1234);
    ::estd::variant<int, bool> b = int(1234);

    EXPECT_FALSE(a < b);
    EXPECT_FALSE(b < a);

    a = int(1);
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);

    a = true;
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(b < a);

    a = false;
    EXPECT_FALSE(a < b);
    EXPECT_TRUE(b < a);
}

TEST(variant, alignment_preservation)
{
    EXPECT_LE(alignof(uint8_t), alignof(::estd::variant<bool, uint8_t>));
    EXPECT_LE(alignof(uint16_t), alignof(::estd::variant<bool, uint16_t>));
    EXPECT_LE(alignof(uint32_t), alignof(::estd::variant<bool, uint32_t>));
    EXPECT_LE(alignof(uint64_t), alignof(::estd::variant<bool, uint64_t>));

    struct alignas(128) A
    {
        uint32_t a;
    };

    EXPECT_EQ(128U, alignof(::estd::variant<bool, A>));
}
