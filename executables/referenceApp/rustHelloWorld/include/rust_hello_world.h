// Copyright 2026 Accenture.

#ifndef RUST_HELLO_WORLD_H
#define RUST_HELLO_WORLD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// Adds two numbers and returns the result.
///
/// \param a First number to add
/// \param b Second number to add
/// \return The sum of a and b
uint32_t rust_add(uint32_t a, uint32_t b);

#ifdef __cplusplus
}
#endif

#endif // RUST_HELLO_WORLD_H
