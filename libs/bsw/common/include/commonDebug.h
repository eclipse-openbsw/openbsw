// Copyright 2024 Accenture.

#pragma once

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#define LOGSYNCHRON(...)                  \
    {                                     \
        printf((char const*)__VA_ARGS__); \
    }
