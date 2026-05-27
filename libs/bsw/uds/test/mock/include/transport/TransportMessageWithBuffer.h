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

#include "transport/TransportMessage.h"

#include <etl/span.h>

#include <cstdint>
#include <vector>

namespace transport
{
namespace test
{
struct TransportMessageWithBuffer
{
    explicit TransportMessageWithBuffer(uint32_t size);
    TransportMessageWithBuffer(
        uint8_t sourceAddress,
        uint8_t targetId,
        ::etl::span<uint8_t const> data,
        uint32_t maxSize = 0);

    TransportMessageWithBuffer(TransportMessageWithBuffer const&)            = delete;
    TransportMessageWithBuffer& operator=(TransportMessageWithBuffer const&) = delete;

    TransportMessage* get() { return &m; }

    TransportMessage& operator*() { return m; }

    TransportMessage* operator->() { return &m; }

private:
    std::vector<uint8_t> buffer;
    TransportMessage m;
};

} // namespace test
} // namespace transport
