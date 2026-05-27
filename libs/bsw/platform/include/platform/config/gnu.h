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

/*
 * Compiler details for the GNU compiler
 */

#define HAS_STDINT_H_
#ifdef __cplusplus
#if __cplusplus < 201103L
#define __STDC_LIMIT_MACROS
#endif

#if __cplusplus >= 201103L
#define HAS_CSTDINT_H_
#endif

#endif
