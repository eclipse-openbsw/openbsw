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

#include "routing/PduRoutingTable.h"

#include <etl/span.h>
#include <etl/unaligned_type.h>
#include <io/IReader.h>
#include <io/IWriter.h>

#include <platform/estdint.h>

namespace routing
{
bool route(
    uint8_t srcChannelId,
    PduRoutingTable const& table,
    ::io::IReader& reader,
    ::etl::span<::io::IWriter*> writers);

void outputPdu(
    ::etl::be_uint32_t outputMessageId,
    uint8_t dstChannelId,
    ::etl::span<uint8_t const> const payload,
    ::io::IWriter& writer);

} // namespace routing
