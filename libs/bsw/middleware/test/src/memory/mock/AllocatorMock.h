/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

#include <etl/array.h>
#include <gmock/gmock.h>

#include "middleware/memory/AllocatorBase.h"

namespace middleware::memory::test
{

class AllocatorMock : public memory::AllocatorBase<AllocatorMock>
{
public:
    using Base                          = memory::AllocatorBase<AllocatorMock>;
    static constexpr size_t MAX_STORAGE = 4096U;

    AllocatorMock(AllocatorMock const& other)            = delete;
    AllocatorMock(AllocatorMock&& other)                 = delete;
    AllocatorMock& operator=(AllocatorMock const& other) = delete;
    AllocatorMock& operator=(AllocatorMock&& other)      = delete;

    MOCK_METHOD(uint8_t*, allocateImpl, (uint32_t const));
    MOCK_METHOD(void, deallocateImpl, (void*));
    MOCK_METHOD(uint8_t*, regionStartImpl, ());
    MOCK_METHOD(bool, isPtrValidImpl, (void const* const));

    static void setAllocatorMock(AllocatorMock& mock);
    static void unsetAllocatorMock();
    static AllocatorMock& getInstance();

private:
    friend testing::NiceMock<AllocatorMock>;

    AllocatorMock();
    ~AllocatorMock();

    uint8_t volatile _dummyMutex{};

    static AllocatorMock* _instance;
    static etl::array<uint8_t, MAX_STORAGE> _storage;
};

} // namespace middleware::memory::test
