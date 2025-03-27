// Copyright 2024 Accenture.

#pragma once

// clang-format off

/*
 * Compiler Identification
 */
// COMPILER_ID_START
#if defined(__clang__)
    #define ESR_COMPILER_CLANG
    #include "platform/config/clang.h" // IWYU pragma: export
#elif defined (__GNUC__)
    #define ESR_COMPILER_GCC
    #include "platform/config/gnu.h" // IWYU pragma: export
#else
    #error "Unsupported compiler"
#endif
// COMPILER_ID_END

#ifdef __GNUC__
    #define ESR_UNUSED __attribute__((unused))
#elif defined(__DCC__)
    // If there is a problem for old diab versions, place
    // #if __VERSION_NUMBER__ >= 5900 ... #endif
    // or similar around the next line.
    #define ESR_UNUSED __attribute__((unused))
#else
    #define ESR_UNUSED
#endif // __GNUC__

#define ESR_NO_INLINE __attribute__ ((noinline))

#if defined(ESR_COMPILER_GCC)     || \
    defined(ESR_COMPILER_CLANG)
    #define ESR_ALWAYS_INLINE __attribute__((always_inline))
#else
    #define ESR_ALWAYS_INLINE
#endif

// clang-format on

#if defined(ESR_COMPILER_CLANG) || defined(ESR_COMPILER_GCC)
#define ESR_PACKED_BEGIN _Pragma("pack(push, 1)")
#define ESR_PACKED_END   _Pragma("pack(pop)")
#endif
