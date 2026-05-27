/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/NullOutputStream.h"

namespace util
{
namespace stream
{
bool NullOutputStream::isEof() const { return true; }

void NullOutputStream::write(uint8_t const) {}

void NullOutputStream::write(::etl::span<uint8_t const> const&) {}

} // namespace stream
} // namespace util
