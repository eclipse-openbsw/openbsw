// Copyright 2024 Accenture.

#ifndef GUARD_9B9AE5E1_2C5E_48AE_80C3_743E956B7FCC
#define GUARD_9B9AE5E1_2C5E_48AE_80C3_743E956B7FCC

#include <platform/estdint.h>

namespace internal
{
class TestClassCalls
{
public:
    TestClassCalls();

    explicit TestClassCalls(int32_t t);
    TestClassCalls(int32_t t1, int32_t t2);
    TestClassCalls(int32_t t1, int32_t t2, int32_t t3);
    TestClassCalls(int32_t t1, int32_t t2, int32_t t3, int32_t t4);
    TestClassCalls(int32_t t1, int32_t t2, int32_t t3, int32_t t4, int32_t t5);

    TestClassCalls(TestClassCalls const& other);
    TestClassCalls(TestClassCalls&& other);

    ~TestClassCalls();

    TestClassCalls& operator=(TestClassCalls const& other);
    TestClassCalls& operator=(TestClassCalls&& other);

    bool operator==(TestClassCalls const& other) const;
    bool operator==(int32_t otherTag) const;

    bool operator<(TestClassCalls const& other) const;

    int32_t tag() const;
    int32_t tag2() const;
    int32_t tag3() const;
    int32_t tag4() const;
    int32_t tag5() const;

    void setTag(int32_t t);

    static void setDebug(bool debug);
    static void reset();

    static bool verify(
        int32_t constructors,
        int32_t copys,
        int32_t moves,
        int32_t destructors,
        int32_t copyAssignments,
        int32_t moveAssignments);

    static int32_t getConstructorCalls();
    static int32_t getCopyCalls();
    static int32_t getMoveCalls();
    static int32_t getDestructorCalls();
    static int32_t getCopyAssignmentCalls();
    static int32_t getMoveAssignmentCalls();

private:
    int32_t _tag;
    int32_t _tag2;
    int32_t _tag3;
    int32_t _tag4;
    int32_t _tag5;

    static int32_t patternDestroyed;
    static int32_t patternMoved;

    static bool debugFlag;
    static int32_t constructorCalls;
    static int32_t copyCalls;
    static int32_t moveCalls;
    static int32_t destructorCalls;
    static int32_t copyAssignmentCalls;
    static int32_t moveAssignmentCalls;
};
} /* namespace internal */

#endif /* GUARD_9B9AE5E1_2C5E_48AE_80C3_743E956B7FCC */
