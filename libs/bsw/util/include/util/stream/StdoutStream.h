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

#include "util/stream/IOutputStream.h"

namespace util
{
namespace stream
{
/**
 * The IOutputStream class implementation.
 *
 */
class StdoutStream : public IOutputStream
{
public:
    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::etl::span<uint8_t const> const& buffer) override;
};

} // namespace stream
} // namespace util
