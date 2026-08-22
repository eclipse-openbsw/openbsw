/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#ifndef RUST_RUNTIME_H
#define RUST_RUNTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void rust_runtime_init(uint8_t task_id);
void rust_runtime_poll(uint8_t task_id);

#ifdef __cplusplus
}
#endif

#endif // RUST_RUNTIME_H
