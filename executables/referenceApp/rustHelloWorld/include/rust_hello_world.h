/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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

/// Prints "Hello from Rust!" to BSP stdout.
void rust_hello_world(void);

#ifdef __cplusplus
}
#endif

#endif // RUST_HELLO_WORLD_H
