// Copyright 2024 Accenture.

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
