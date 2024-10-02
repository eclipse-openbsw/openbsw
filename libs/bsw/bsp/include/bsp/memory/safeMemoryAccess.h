// Copyright 2024 Accenture.

#ifndef GUARD_ECA56EE9_6D4D_4A4C_BA1A_60DC03035E96
#define GUARD_ECA56EE9_6D4D_4A4C_BA1A_60DC03035E96

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

#endif /* GUARD_ECA56EE9_6D4D_4A4C_BA1A_60DC03035E96 */
