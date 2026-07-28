/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once

#include <etl/error_handler.h>

namespace middleware::util
{
/**
 * \brief CRTP base class that wraps a GMock class
 *
 * The getMock() function provides access to the mock object and its mocked
 * functions. Free mock functions can use this to redirect concrete
 * implementations to the mock class T.
 */
template<typename T>
class CrtpMock
{
public:
    ~CrtpMock() { gInstance = nullptr; }

    static bool isInstantiated() { return gInstance != nullptr; }

    static auto* getMock()
    {
        ETL_ASSERT(gInstance != nullptr, ETL_ERROR_GENERIC("CrtpMock instance not set."));
        return gInstance;
    }

private:
    static inline T* gInstance;

    CrtpMock() { gInstance = static_cast<T*>(this); }

    // Explicitly disable copy/move to satisfy special member function guidelines
    CrtpMock(CrtpMock const&)            = delete;
    CrtpMock& operator=(CrtpMock const&) = delete;
    CrtpMock(CrtpMock&&)                 = delete;
    CrtpMock& operator=(CrtpMock&&)      = delete;

    friend T;
};

} // namespace middleware::util
