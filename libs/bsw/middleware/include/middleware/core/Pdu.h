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

#include <etl/array.h>
#include <etl/atomic.h>
#include <cstdint>

namespace middleware::core
{

template<size_t SIZE>
struct Pdu
{
    etl::atomic<size_t> seqCounter{};
    etl::array<uint8_t, SIZE> data{};
};

} // namespace middleware::core
