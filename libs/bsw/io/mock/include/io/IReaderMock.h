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

#include "io/IReader.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace io
{
class IReaderMock : public IReader
{
public:
    MOCK_METHOD(size_t, maxSize, (), (const));

    MOCK_METHOD(::etl::span<uint8_t>, peek, (), (const));

    MOCK_METHOD(void, release, ());
};

} // namespace io
