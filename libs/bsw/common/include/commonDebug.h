// Copyright 2024 Accenture.

#ifndef GUARD_B472AE56_68C0_4CB3_A88E_81E436C60407
#define GUARD_B472AE56_68C0_4CB3_A88E_81E436C60407

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#define LOGSYNCHRON(...)                  \
    {                                     \
        printf((char const*)__VA_ARGS__); \
    }

#endif /* GUARD_B472AE56_68C0_4CB3_A88E_81E436C60407 */
