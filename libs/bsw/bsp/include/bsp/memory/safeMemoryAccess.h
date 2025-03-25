// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum SafeReadStatus
{
    READ_FAILED,
    READ_SUCCEEDED
};

SafeReadStatus safeRead(uint32_t const* address, uint32_t* value);
SafeReadStatus safeRead64(uint64_t const* address, uint64_t* value);
SafeReadStatus safeBlockRead(uint8_t const* src, uint8_t* dst, uint32_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

