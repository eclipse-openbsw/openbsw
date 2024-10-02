// Copyright 2024 Accenture.

#include "bsp/charInputOutput/Bspio.h"

#include "charInputOutput/charIo.h"

#include <printf/printf.h>

#include <cstring>
#include <limits>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \par Linking this module
 * The diab compiler is providing these methods for char in-/output. They will
 * be used per default. To use the compiler's printf/scanf functions for debug
 * in-/output, the defaults have to be overwritten. This can be achieved by
 * linking this module's object, e.g: ld -l:src/bios/bspIO/chario.o
 *
 */
int __inchar(void) { return (charIO__inchar()); }
int __inedit(void) { return __inchar(); }
int __outchar(int const c, int const last) { return charIO__outchar(c, last); }
int __outedit(int const c, int const last)
{
    if (c == 0xA)
    {
        (void)__outchar(0xD, last);
    }
    return __outchar(c, last);
}

int vsnprintf(char* buf, const size_t maxsize, const char* fmt, va_list args)
{
    return vsnprintf_(buf, maxsize, fmt, args);
}

int vsprintf(char* buf, const char* fmt, va_list args) { return vsprintf_(buf, fmt, args); }

int snprintf(char* buf, size_t maxsize, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto const ret = vsnprintf_(buf, maxsize, fmt, args);
    va_end(args);
    return ret;
}

int sprintf(char* buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto const ret = vsprintf_(buf, fmt, args);
    va_end(args);
    return ret;
}

void putchar_(char const character)
{
    // __outedit takes an int - cast to unsigned before passing along
    unsigned char const u_char = static_cast<unsigned char>(character);
    (void)__outedit(static_cast<int>(u_char), 0);
}

void fctprintf_helper(char c, void* /* satisfy fctprintf outFunc reqs */)
{
    // __outedit takes an int - cast to unsigned before passing along
    unsigned char const u_char = static_cast<unsigned char>(c);
    (void)__outedit(static_cast<int>(u_char), 0);
}

uint8_t debug_printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    (void)vfctprintf(&fctprintf_helper, nullptr, reinterpret_cast<char const*>(format), args);
    va_end(args);

    return 0;
}

#include "charInputOutput/printfPragma.hpp"

#ifdef __cplusplus
} // extern "C"
#endif
