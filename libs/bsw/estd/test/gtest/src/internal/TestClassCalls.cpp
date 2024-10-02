// Copyright 2024 Accenture.

#include "internal/TestClassCalls.h"

#include <gtest/gtest.h>

#include <cstdio>

namespace internal
{
// static
int32_t TestClassCalls::patternDestroyed
    = static_cast<int32_t>(::std::strtol("0xDEDEDEDE", nullptr, 16));

// static
int32_t TestClassCalls::patternMoved
    = static_cast<int32_t>(::std::strtol("0xAAAAAAAA", nullptr, 16));

// static
bool TestClassCalls::debugFlag = false;

// static
int32_t TestClassCalls::constructorCalls = 0;

// static
int32_t TestClassCalls::copyCalls = 0;

// static
int32_t TestClassCalls::moveCalls = 0;

// static
int32_t TestClassCalls::destructorCalls = 0;

// static
int32_t TestClassCalls::copyAssignmentCalls = 0;

// static
int32_t TestClassCalls::moveAssignmentCalls = 0;

// static
void TestClassCalls::reset()
{
    constructorCalls    = 0;
    copyCalls           = 0;
    moveCalls           = 0;
    destructorCalls     = 0;
    copyAssignmentCalls = 0;
    moveAssignmentCalls = 0;
}

// static
bool TestClassCalls::verify(
    int32_t constructors,
    int32_t copys,
    int32_t moves,
    int32_t destructors,
    int32_t copyAssignments,
    int32_t moveAssignments)
{
    bool constructorsEq = true;
    if (constructors != -1)
    {
        constructorsEq = (constructors == constructorCalls);
        EXPECT_EQ(constructors, constructorCalls);
    }
    bool copysEq = true;
    if (copys != -1)
    {
        copysEq = (copys == copyCalls);
        EXPECT_EQ(copys, copyCalls);
    }
    bool movesEq = true;
    if (moves != -1)
    {
        movesEq = (moves == moveCalls);
        EXPECT_EQ(moves, moveCalls);
    }
    bool destructorsEq = true;
    if (destructors != -1)
    {
        destructorsEq = (destructors == destructorCalls);
        EXPECT_EQ(destructors, destructorCalls);
    }
    bool copyAssignmentsEq = true;
    if (copyAssignments != -1)
    {
        copyAssignmentsEq = (copyAssignments == copyAssignmentCalls);
        EXPECT_EQ(copyAssignments, copyAssignmentCalls);
    }
    bool moveAssignmentsEq = true;
    if (moveAssignments != -1)
    {
        moveAssignmentsEq = (moveAssignments == moveAssignmentCalls);
        EXPECT_EQ(moveAssignments, moveAssignmentCalls);
    }

    return constructorsEq && copysEq && movesEq && destructorsEq && copyAssignmentsEq
           && moveAssignmentsEq;
}

TestClassCalls::TestClassCalls() : _tag(0), _tag2(-1), _tag3(-1), _tag4(-1), _tag5(-1)
{
    if (debugFlag)
    {
        fprintf(stderr, "Default Constructor\n");
    }

    constructorCalls++;
}

TestClassCalls::TestClassCalls(int32_t t) : _tag(t), _tag2(-1), _tag3(-1), _tag4(-1), _tag5(-1)
{
    if (debugFlag)
    {
        fprintf(stderr, "Constructor: tag=%d\n", t);
    }

    constructorCalls++;
}

TestClassCalls::TestClassCalls(int32_t t, int32_t tag2)
: _tag(t), _tag2(tag2), _tag3(-1), _tag4(-1), _tag5(-1)
{
    if (debugFlag)
    {
        fprintf(stderr, "Constructor: tag=%d\n", t);
    }

    constructorCalls++;
}

TestClassCalls::TestClassCalls(int32_t t, int32_t tag2, int32_t tag3)
: _tag(t), _tag2(tag2), _tag3(tag3), _tag4(-1), _tag5(-1)
{
    if (debugFlag)
    {
        fprintf(stderr, "Constructor: tag=%d\n", t);
    }

    constructorCalls++;
}

TestClassCalls::TestClassCalls(int32_t t, int32_t tag2, int32_t tag3, int32_t tag4)
: _tag(t), _tag2(tag2), _tag3(tag3), _tag4(tag4), _tag5(-1)
{
    if (debugFlag)
    {
        fprintf(stderr, "Constructor: tag=%d\n", t);
    }

    constructorCalls++;
}

TestClassCalls::TestClassCalls(int32_t t, int32_t tag2, int32_t tag3, int32_t tag4, int32_t tag5)
: _tag(t), _tag2(tag2), _tag3(tag3), _tag4(tag4), _tag5(tag5)
{
    if (debugFlag)
    {
        fprintf(stderr, "Constructor: tag=%d\n", t);
    }

    constructorCalls++;
}

TestClassCalls::TestClassCalls(TestClassCalls const& other)
: _tag(other._tag), _tag2(other._tag2), _tag3(other._tag3), _tag4(other._tag4), _tag5(other._tag5)
{
    if (debugFlag)
    {
        fprintf(stderr, "Copy Constructor: tag=%d\n", _tag);
    }
    copyCalls++;
}

TestClassCalls::TestClassCalls(TestClassCalls&& other)
: _tag(other._tag), _tag2(other._tag2), _tag3(other._tag3), _tag4(other._tag4), _tag5(other._tag5)
{
    if (debugFlag)
    {
        fprintf(stderr, "Move Constructor: tag=%d\n", _tag);
    }
    other._tag = other._tag2 = other._tag3 = other._tag4 = other._tag5 = patternMoved;
    moveCalls++;
}

TestClassCalls::~TestClassCalls()
{
    if (debugFlag)
    {
        fprintf(stderr, "Destructor: tag=%d\n", _tag);
    }
    _tag = _tag2 = _tag3 = _tag4 = _tag5 = patternDestroyed;
    destructorCalls++;
}

TestClassCalls& TestClassCalls::operator=(TestClassCalls const& other)
{
    if (debugFlag)
    {
        fprintf(stderr, "Copy Assignment: oldTag=%d newTag=%d\n", _tag, other._tag);
    }
    _tag  = other._tag;
    _tag2 = other._tag2;
    _tag3 = other._tag3;
    _tag4 = other._tag4;
    _tag5 = other._tag5;

    copyAssignmentCalls++;
    return *this;
}

TestClassCalls& TestClassCalls::operator=(TestClassCalls&& other)
{
    if (debugFlag)
    {
        fprintf(stderr, "Move Assignment: oldTag=%d newTag=%d\n", _tag, other._tag);
    }
    _tag  = other._tag;
    _tag2 = other._tag2;
    _tag3 = other._tag3;
    _tag4 = other._tag4;
    _tag5 = other._tag5;

    other._tag = other._tag2 = other._tag3 = other._tag4 = other._tag5 = patternMoved;

    moveAssignmentCalls++;
    return *this;
}

bool TestClassCalls::operator==(TestClassCalls const& other) const { return _tag == other._tag; }

bool TestClassCalls::operator==(int32_t otherTag) const { return _tag == otherTag; }

bool TestClassCalls::operator<(TestClassCalls const& other) const { return _tag < other._tag; }

int32_t TestClassCalls::tag() const { return _tag; }

int32_t TestClassCalls::tag2() const { return _tag2; }

int32_t TestClassCalls::tag3() const { return _tag3; }

int32_t TestClassCalls::tag4() const { return _tag4; }

int32_t TestClassCalls::tag5() const { return _tag5; }

void TestClassCalls::setTag(int32_t t) { _tag = t; }

// static
void TestClassCalls::setDebug(bool debug) { debugFlag = debug; }

// static
int32_t TestClassCalls::getConstructorCalls() { return constructorCalls; }

// static
int32_t TestClassCalls::getCopyCalls() { return copyCalls; }

// static
int32_t TestClassCalls::getMoveCalls() { return moveCalls; }

// static
int32_t TestClassCalls::getDestructorCalls() { return destructorCalls; }

// static
int32_t TestClassCalls::getCopyAssignmentCalls() { return copyAssignmentCalls; }

// static
int32_t TestClassCalls::getMoveAssignmentCalls() { return moveAssignmentCalls; }

} /* namespace internal */
