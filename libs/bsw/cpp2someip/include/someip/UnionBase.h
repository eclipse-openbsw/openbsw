/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/ISomeIpSerializable.h"

#include <cstring>

namespace someip
{
class UnionBase : public ISomeIpSerializable
{
protected:
    template<typename T>
    static T const* castAsConstUnionType(uint8_t const* data);

    template<typename T>
    static T* castAsUnionType(uint8_t* data);

    static void memCpy(void* toData, void const* fromData, size_t size);
};

/*
 * inline implementation
 */

// static
template<typename T>
inline T const* UnionBase::castAsConstUnionType(uint8_t const* const data)
{
    return reinterpret_cast<T const*>(data);
}

// static
template<typename T>
inline T* UnionBase::castAsUnionType(uint8_t* const data)
{
    return reinterpret_cast<T*>(data);
}

// static
inline void UnionBase::memCpy(void* const toData, void const* const fromData, size_t const size)
{
    (void)::memcpy(toData, fromData, size);
}

} // namespace someip
