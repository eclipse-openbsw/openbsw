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

#include "util/crc/Crc.h"

namespace util
{
namespace crc
{
struct Crc16
{
    using Ccitt = CrcRegister<uint16_t, 0x1021, 0xFFFF>;
};

} /* namespace crc */
} /* namespace util */
