// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
namespace safe_memory
{
bool checkRamDoubleBitError();
bool checkFlashDoubleBitError();
uint32_t readErmMemoryErrorAddress();
} // namespace safe_memory
} // namespace safety
