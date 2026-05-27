/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

enum MyEvent
{
    SOMETHING_HAPPENED
};

class HandlerMock
{
public:
    MOCK_METHOD(void, handle, (MyEvent const& event));
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
