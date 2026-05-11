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

/**
 * This file defines and provides functions to interact with the blob's header.
 */

#include <etl/span.h>

namespace blob
{

struct Header
{
    uint32_t version = 0U;
    uint32_t magic   = 0U;
    uint32_t size    = 0U;
};

/**
 * Given blob span, checks the blob's version and magic number.
 */
// [HeaderCheck]
bool checkHeader(::etl::span<uint8_t const> blob, Header& header);
// [HeaderCheck]

} // namespace blob
