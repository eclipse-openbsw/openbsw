// Copyright 2024 Accenture.

// Include class we are testing first to check for include dependencies
#include "estd/constructor.h"

#include "estd/uncopyable.h"
#include "internal/TestClassCalls.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using TestObject = ::internal::TestClassCalls;

bool validate(TestObject const& other, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    EXPECT_EQ(other.tag(), p1);
    EXPECT_EQ(other.tag2(), p2);
    EXPECT_EQ(other.tag3(), p3);
    EXPECT_EQ(other.tag4(), p4);
    EXPECT_EQ(other.tag5(), p5);

    return (other.tag() == p1) && (other.tag2() == p2) && (other.tag3() == p3)
           && (other.tag4() == p4) && (other.tag5() == p5);
}

TEST(Constructor, has_memory)
{
    uint8_t data[sizeof(int32_t)];
    ::estd::constructor<int32_t> c(data);
    EXPECT_TRUE(c.has_memory());
}

TEST(Constructor, has_no_memory)
{
    ::estd::constructor<int32_t> c(nullptr);
    EXPECT_FALSE(c.has_memory());
}

TEST(Constructor, ReleaseReturnsCorrectAddress)
{
    uint8_t data[sizeof(TestObject)];
    ::estd::constructor<TestObject> c(data);

    ASSERT_EQ(&data[0], c.release());
}

TEST(Constructor, OperatorCastToPointerReturnsCorrectAddress)
{
    uint8_t data[sizeof(TestObject)];
    ::estd::constructor<TestObject> c(data);

    ASSERT_EQ(&data[0], static_cast<uint8_t*>(c));
}

TEST(Constructor, TestDefaultConstructor)
{
    uint8_t data[sizeof(TestObject)];

    ::estd::constructor<TestObject> c(data);

    TestObject& tmp = c.construct();
    ASSERT_TRUE(validate(tmp, 0, -1, -1, -1, -1));
}

class ConstructorClass
{
public:
    int32_t _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9;

    ConstructorClass(int32_t p1)
    : _p1(p1), _p2(-2), _p3(-3), _p4(-4), _p5(-5), _p6(-6), _p7(-7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(int32_t p1, int32_t p2)
    : _p1(p1), _p2(p2), _p3(-3), _p4(-4), _p5(-5), _p6(-6), _p7(-7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(int32_t p1, int32_t p2, int32_t p3)
    : _p1(p1), _p2(p2), _p3(p3), _p4(-4), _p5(-5), _p6(-6), _p7(-7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(int32_t p1, int32_t p2, int32_t p3, int32_t p4)
    : _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(-5), _p6(-6), _p7(-7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
    : _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(-6), _p7(-7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
    : _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(p6), _p7(-7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(
        int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
    : _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(p6), _p7(p7), _p8(-8), _p9(-9)
    {}

    ConstructorClass(
        int32_t p1,
        int32_t p2,
        int32_t p3,
        int32_t p4,
        int32_t p5,
        int32_t p6,
        int32_t p7,
        int32_t p8)
    : _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(p6), _p7(p7), _p8(p8), _p9(-9)
    {}

    ConstructorClass(
        int32_t p1,
        int32_t p2,
        int32_t p3,
        int32_t p4,
        int32_t p5,
        int32_t p6,
        int32_t p7,
        int32_t p8,
        int32_t p9)
    : _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(p6), _p7(p7), _p8(p8), _p9(p9)
    {}
};

TEST(Constructor, TestOneConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(-2, tmp._p2);
    ASSERT_EQ(-3, tmp._p3);
    ASSERT_EQ(-4, tmp._p4);
    ASSERT_EQ(-5, tmp._p5);
    ASSERT_EQ(-6, tmp._p6);
    ASSERT_EQ(-7, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestTwoConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(-3, tmp._p3);
    ASSERT_EQ(-4, tmp._p4);
    ASSERT_EQ(-5, tmp._p5);
    ASSERT_EQ(-6, tmp._p6);
    ASSERT_EQ(-7, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestThreeConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(-4, tmp._p4);
    ASSERT_EQ(-5, tmp._p5);
    ASSERT_EQ(-6, tmp._p6);
    ASSERT_EQ(-7, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestFourConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30, 40);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(40, tmp._p4);
    ASSERT_EQ(-5, tmp._p5);
    ASSERT_EQ(-6, tmp._p6);
    ASSERT_EQ(-7, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestFiveConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30, 40, 50);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(40, tmp._p4);
    ASSERT_EQ(50, tmp._p5);
    ASSERT_EQ(-6, tmp._p6);
    ASSERT_EQ(-7, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestSixConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30, 40, 50, 60);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(40, tmp._p4);
    ASSERT_EQ(50, tmp._p5);
    ASSERT_EQ(60, tmp._p6);
    ASSERT_EQ(-7, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestSevenConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30, 40, 50, 60, 70);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(40, tmp._p4);
    ASSERT_EQ(50, tmp._p5);
    ASSERT_EQ(60, tmp._p6);
    ASSERT_EQ(70, tmp._p7);
    ASSERT_EQ(-8, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestEightConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30, 40, 50, 60, 70, 80);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(40, tmp._p4);
    ASSERT_EQ(50, tmp._p5);
    ASSERT_EQ(60, tmp._p6);
    ASSERT_EQ(70, tmp._p7);
    ASSERT_EQ(80, tmp._p8);
    ASSERT_EQ(-9, tmp._p9);
}

TEST(Constructor, TestNineConstructor)
{
    uint8_t data[sizeof(ConstructorClass)];

    ::estd::constructor<ConstructorClass> c(data);

    ConstructorClass& tmp = c.construct(10, 20, 30, 40, 50, 60, 70, 80, 90);
    ASSERT_EQ(10, tmp._p1);
    ASSERT_EQ(20, tmp._p2);
    ASSERT_EQ(30, tmp._p3);
    ASSERT_EQ(40, tmp._p4);
    ASSERT_EQ(50, tmp._p5);
    ASSERT_EQ(60, tmp._p6);
    ASSERT_EQ(70, tmp._p7);
    ASSERT_EQ(80, tmp._p8);
    ASSERT_EQ(90, tmp._p9);
}

TEST(Constructor, TestCopyConstructor)
{
    uint8_t data[sizeof(TestObject)];

    ::estd::constructor<TestObject> c(data);

    TestObject a(1, 2, 3, 4, 5);

    TestObject& tmp = c.construct(a);
    ASSERT_TRUE(validate(tmp, 1, 2, 3, 4, 5));
}

class TestClassNonIntegral
{
public:
    TestClassNonIntegral(::internal::TestClassCalls t) : _t(std::move(t)) {}

    ::internal::TestClassCalls& data() { return _t; }

private:
    ::internal::TestClassCalls _t;
};

TEST(Constructor, TestConstructorCalls)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    uint8_t buffer[sizeof(TestClassCalls)];
    TestClassCalls& t = ::estd::constructor<TestClassCalls>(buffer).construct(10);
    ASSERT_EQ(10, t.tag());
    ASSERT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
}

TEST(Constructor, TestCopyConstructorCalls)
{
    using ::internal::TestClassCalls;
    TestClassCalls t(10);
    TestClassCalls::reset();
    uint8_t buffer[sizeof(TestClassCalls)];
    TestClassCalls& c = ::estd::constructor<TestClassCalls>(buffer).construct(t);
    ASSERT_EQ(10, c.tag());
    ASSERT_TRUE(TestClassCalls::verify(0, 1, 0, 0, 0, 0));
}

TEST(Constructor, TestMoveConstructorCalls)
{
    using ::internal::TestClassCalls;
    TestClassCalls t(10);
    TestClassCalls::reset();
    uint8_t buffer[sizeof(TestClassCalls)];
    TestClassCalls& c = ::estd::constructor<TestClassCalls>(buffer).construct(::std::move(t));
    ASSERT_EQ(10, c.tag());
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 1, 0, 0, 0));
}

TEST(Constructor, TestMemberConstructorCalls)
{
    using ::internal::TestClassCalls;

    TestClassCalls t(10);
    TestClassCalls::reset();
    uint8_t buffer[sizeof(TestClassNonIntegral)];
    TestClassNonIntegral& c = ::estd::constructor<TestClassNonIntegral>(buffer).construct(t);
    ASSERT_EQ(10, c.data().tag());
    ASSERT_TRUE(TestClassCalls::verify(0, 1, 1, 1, 0, 0));
}

TEST(Constructor, TestMemberConstructorCallsMove)
{
    using ::internal::TestClassCalls;

    TestClassCalls t(10);
    TestClassCalls::reset();
    uint8_t buffer[sizeof(TestClassNonIntegral)];
    TestClassNonIntegral& c
        = ::estd::constructor<TestClassNonIntegral>(buffer).construct(::std::move(t));
    ASSERT_EQ(10, c.data().tag());
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 2, 1, 0, 0));
}

class ConstructSimpleByRef
{
public:
    ConstructSimpleByRef(uint8_t& total) : _total(total) { _total++; }

    uint8_t& _total;
};

TEST(Constructor, ConstructSimpleByRef)
{
    uint8_t buffer[sizeof(ConstructSimpleByRef)];
    uint8_t total     = 10;
    uint8_t& totalRef = total;
    ConstructSimpleByRef& obj
        = ::estd::constructor<ConstructSimpleByRef>(buffer).construct(::estd::by_ref(totalRef));

    ASSERT_EQ(11U, obj._total);
    ASSERT_EQ(11U, total);
}

class ConstructSimpleByValue
{
public:
    ConstructSimpleByValue(uint8_t total) : _total(total) { _total++; }

    uint8_t _total;
};

TEST(Constructor, ConstructSimpleByValue)
{
    uint8_t buffer[sizeof(ConstructSimpleByValue)];
    uint8_t total = 10;
    ConstructSimpleByValue& obj
        = ::estd::constructor<ConstructSimpleByValue>(buffer).construct(total);

    ASSERT_EQ(11U, obj._total);
    ASSERT_EQ(10U, total);
}

class LocalComplex
{
public:
    LocalComplex(uint8_t a, uint16_t b, uint32_t c) : _uint8(a), _uint16(b), _uint32(c) {}

    uint8_t _uint8;
    uint16_t _uint16;
    uint32_t _uint32;
};

class ConstructComplexByRef
{
    UNCOPYABLE(ConstructComplexByRef);

public:
    ConstructComplexByRef(LocalComplex& obj) : _obj(obj)
    {
        ++_obj._uint8;
        ++_obj._uint16;
        ++_obj._uint32;
    }

    LocalComplex& _obj;
};

TEST(Constructor, ConstructComplexByRef)
{
    uint8_t buffer[sizeof(ConstructComplexByRef)];
    LocalComplex complex(1, 2, 3);
    ConstructComplexByRef& obj
        = ::estd::constructor<ConstructComplexByRef>(buffer).construct(::estd::by_ref(complex));

    ASSERT_EQ(2U, obj._obj._uint8);
    ASSERT_EQ(3U, obj._obj._uint16);
    ASSERT_EQ(4U, obj._obj._uint32);
}
} // namespace
