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

namespace util
{
namespace crc
{
// FIXME: add a static assert for the non specialized variant
//        so the error message shows that the crc table is missing!

template<class T, T Polynom>
struct LookupTable
{
    using TableRef = T const (&)[256];
    static TableRef getTable();
};

} // namespace crc
} // namespace util
