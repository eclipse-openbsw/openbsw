#pragma once

#include "platform/config.h"
#include "platform/estdint.h"

#include <cstddef>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define is_digit(c) (((c) >= '0') && ((c) <= '9'))

#define ZEROPAD 1  /* pad with zero */
#define SIGN    2  /* unsigned/signed long */
#define PLUS    4  /* show plus */
#define SPACE   8  /* space if plus */
#define LEFT    16 /* left justified */
#define SPECIAL 32 /* 0x */
#define LARGE   64 /* use 'ABCDEF' instead of 'abcdef' */

#define do_div(n, base) do_div_hlp(&(n), (base))

#if defined(__DCC__)
// clang-format off
#pragma inline do_div_hlp
// clang-format on
#endif
ESR_UNUSED static int do_div_hlp(unsigned long* const n, int const base)
{
    int const res = (int)(*n % (unsigned long)base);

    *n = *n / (unsigned long)base;

    return res;
}

/**
 *
 * \brief Print to the supplied buffer.
 *
 * This function formats the optional arguments according to the
 * printf()-like format string \a fmt and stores the resulting string in
 * buffer \a buf.
 *
 * \param buf Buffer to store string in. (IN)
 * \param fmt printf()-like format string. (IN)
 *
 * \return Length of the resulting string.
 *
 */
extern int sprintf(char* buf, char const* fmt, ...);

/**
 *
 * \brief Print to the supplied buffer.
 *
 * This function formats the optional arguments according to the
 * printf()-like format string \a fmt and stores the resulting string in
 * buffer \a buf.
 *
 * \param buf Buffer to store string in. (IN)
 * \param maxsize max Size from buf
 * \param fmt printf()-like format string. (IN)
 *
 * \return Length of the resulting string.
 *
 */
extern int snprintf(char* buf, size_t const maxsize, char const* fmt, ...);

/**
 *
 * \brief Print to the supplied buffer.
 *
 * This function formats the arguments \a args according to the
 * printf()-like format string \a fmt and stores the resulting string in
 * buffer \a buf.
 *
 * \param buf Buffer to store string in. (IN)
 * \param fmt printf()-like format string. (IN)
 * \param args Arguments to format. (IN)
 *
 * \return Length of the resulting string.
 *
 */
extern int vsprintf(char* buf, char const* fmt, va_list args);

/**
 *
 * \brief Print to the supplied buffer.
 *
 * This function formats the arguments \a args according to the
 * printf()-like format string \a fmt and stores the resulting string in
 * buffer \a buf.
 *
 * \param buf Buffer to store string in. (IN)
 * \param maxsize max Size from buf* *
 * \param fmt printf()-like format string. (IN)
 * \param args Arguments to format. (IN)
 *
 * \return Length of the resulting string.
 *
 */
extern int vsnprintf(char* buf, size_t const maxsize, char const* fmt, va_list args);

#ifdef __cplusplus
}
#endif

