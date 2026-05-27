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

#include "util/stream/INonBlockingInputStream.h"

namespace util
{
namespace stream
{
/**
 * The INonBlockingInputStream class implementation.
 *
 */
class StdinStream : public INonBlockingInputStream
{
public:
    bool isEof() const override;
    uint32_t readBuffer(::etl::span<uint8_t> const& buffer) override;
};

} // namespace stream
} // namespace util
