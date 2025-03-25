// Copyright 2025 Accenture.

#pragma once

#include <gmock/gmock.h>

enum MyEvent
{
    SOMETHING_HAPPENED
};

class HandlerMock
{
public:
    MOCK_METHOD1(handle, void(MyEvent const& event));
};

class ScopedMutexMock
{
public:
    ScopedMutexMock();
    ~ScopedMutexMock();
    static void reset();
    static int numConstructed();
    static bool allDestructed();

private:
    static int _numConstructed;
    static int _numDestructed;
};

struct MyContext
{
    unsigned int value = 0xDEADBEEF;
};

