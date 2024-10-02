// Copyright 2024 Accenture.

/*
 * \ingroup dlt
 */

#ifndef GUARD_77C21493_4E81_4360_B34E_B7960EA12621
#define GUARD_77C21493_4E81_4360_B34E_B7960EA12621

#include "util/preprocessor/Macros.h"
#include "util/string/ConstString.h"

#include <estd/slice.h>

// logging interfaces
#ifndef LOG_LEVEL_OFF
#define LOG_LEVEL_OFF 5
#endif // LOG_LEVEL_OFF

#ifndef LOG_LEVEL_FATAL
#define LOG_LEVEL_FATAL 4
#endif // LOG_LEVEL_FATAL

#ifndef LOG_LEVEL_ERROR
#define LOG_LEVEL_ERROR 3
#endif // LOG_LEVEL_ERROR

#ifndef LOG_LEVEL_WARN
#define LOG_LEVEL_WARN 2
#endif // LOG_LEVEL_WARN

#ifndef LOG_LEVEL_INFO
#define LOG_LEVEL_INFO 1
#endif // LOG_LEVEL_INFO

#ifndef LOG_LEVEL_DEBUG
#define LOG_LEVEL_DEBUG 0
#endif // LOG_LEVEL_DEBUG

#ifndef LOG_F
#define LOG_F(COMPONENT, MESSAGE, ...) ESR_CHECK_LOG(COMPONENT, MESSAGE, __VA_ARGS__)
#endif // LOG_F

#ifndef LOG_FMT_F
#define LOG_FMT_F(COMPONENT, FORMAT, ...) ESR_CHECK_LOG(COMPONENT, FORMAT, __VA_ARGS__)
#endif // LOG_FMT_F

#ifndef LOG_E
#define LOG_E(COMPONENT, MESSAGE, ...) ESR_CHECK_LOG(COMPONENT, MESSAGE, __VA_ARGS__)
#endif // LOG_E

#ifndef LOG_FMT_E
#define LOG_FMT_E(COMPONENT, FORMAT, ...) ESR_CHECK_LOG(COMPONENT, FORMAT, __VA_ARGS__)
#endif // LOG_FMT_E

#ifndef LOG_W
#define LOG_W(COMPONENT, MESSAGE, ...) ESR_CHECK_LOG(COMPONENT, MESSAGE, __VA_ARGS__)
#endif // LOG_W

#ifndef LOG_FMT_W
#define LOG_FMT_W(COMPONENT, FORMAT, ...) ESR_CHECK_LOG(COMPONENT, FORMAT, __VA_ARGS__)
#endif // LOG_FMT_W

#ifndef LOG_I
#define LOG_I(COMPONENT, MESSAGE, ...) ESR_CHECK_LOG(COMPONENT, MESSAGE, __VA_ARGS__)
#endif // LOG_I

#ifndef LOG_FMT_I
#define LOG_FMT_I(COMPONENT, FORMAT, ...) ESR_CHECK_LOG(COMPONENT, FORMAT, __VA_ARGS__)
#endif // LOG_FMT_I

#ifndef LOG_D
#define LOG_D(COMPONENT, MESSAGE, ...) ESR_CHECK_LOG(COMPONENT, MESSAGE, __VA_ARGS__)
#endif // LOG_D

#ifndef LOG_FMT_D
#define LOG_FMT_D(COMPONENT, FORMAT, ...) ESR_CHECK_LOG(COMPONENT, FORMAT, __VA_ARGS__)
#endif // LOG_FMT_D

#ifndef LOG_X
#define LOG_X(COMPONENT, LEVEL, MESSAGE, ...) \
    ESR_CHECK_ARG(uint8_t, LEVEL)             \
    ESR_CHECK_LOG(COMPONENT, MESSAGE, __VA_ARGS__)
#endif // LOG_X

#ifndef LOG_FMT_X
#define LOG_FMT_X(COMPONENT, LEVEL, FORMAT, ...) \
    ESR_CHECK_ARG(uint8_t, LEVEL)                \
    ESR_CHECK_LOG(COMPONENT, FORMAT, __VA_ARGS__)
#endif // LOG_FMT_X

#ifndef L_BOOL
#define L_BOOL(VAL) ESR_CHECK_ARG(bool, VAL)
#endif // L_BOOL

#ifndef L_I8
#define L_I8(VAL) ESR_CHECK_ARG(int8_t, VAL)
#endif // L_I8

#ifndef L_U8
#define L_U8(VAL) ESR_CHECK_ARG(uint8_t, VAL)
#endif // L_U8

#ifndef L_H8
#define L_H8(VAL) ESR_CHECK_ARG(uint8_t, VAL)
#endif // L_H8

#ifndef L_I16
#define L_I16(VAL) ESR_CHECK_ARG(int16_t, VAL)
#endif // L_I16

#ifndef L_U16
#define L_U16(VAL) ESR_CHECK_ARG(uint16_t, VAL)
#endif // L_U16

#ifndef L_H16
#define L_H16(VAL) ESR_CHECK_ARG(uint16_t, VAL)
#endif // L_H16

#ifndef L_I32
#define L_I32(VAL) ESR_CHECK_ARG(int32_t, VAL)
#endif // L_I32

#ifndef L_U32
#define L_U32(VAL) ESR_CHECK_ARG(uint32_t, VAL)
#endif // L_U32

#ifndef L_H32
#define L_H32(VAL) ESR_CHECK_ARG(uint32_t, VAL)
#endif // L_H32

#ifndef L_I64
#define L_I64(VAL) ESR_CHECK_ARG(int64_t, VAL)
#endif // L_I64

#ifndef L_U64
#define L_U64(VAL) ESR_CHECK_ARG(uint64_t, VAL)
#endif // L_U64

#ifndef L_H64
#define L_H64(VAL) ESR_CHECK_ARG(uint64_t, VAL)
#endif // L_H64

#ifndef L_STR
#define L_STR(VAL) ESR_CHECK_ARG(char const*, VAL)
#endif // L_STR

#ifndef L_SIZED_STR
#define L_SIZED_STR(VAL) ESR_CHECK_ARG(::util::string::ConstString, VAL)
#endif // L_SIZED_STR

#ifndef L_PTR
#define L_PTR(VAL) ESR_CHECK_ARG(uint8_t const*, reinterpret_cast<uint8_t const*>(VAL))
#endif // L_PTR

#ifndef L_ARRAY_BOOL
#define L_ARRAY_BOOL(VAL) ESR_CHECK_ARG(::estd::slice<bool const>, VAL)
#endif // L_ARRAY_BOOL

#ifndef L_ARRAY_I8
#define L_ARRAY_I8(VAL) ESR_CHECK_ARG(::estd::slice<const int8_t>, VAL)
#endif // L_ARRAY_I8

#ifndef L_ARRAY_U8
#define L_ARRAY_U8(VAL) ESR_CHECK_ARG(::estd::slice<uint8_t const>, VAL)
#endif // L_ARRAY_U8

#ifndef L_ARRAY_H8
#define L_ARRAY_H8(VAL) ESR_CHECK_ARG(::estd::slice<uint8_t const>, VAL)
#endif // L_ARRAY_H8

#ifndef L_ARRAY_I16
#define L_ARRAY_I16(VAL) ESR_CHECK_ARG(::estd::slice<const int16_t>, VAL)
#endif // L_ARRAY_I16

#ifndef L_ARRAY_U16
#define L_ARRAY_U16(VAL) ESR_CHECK_ARG(::estd::slice<const uint16_t>, VAL)
#endif // L_ARRAY_U16

#ifndef L_ARRAY_H16
#define L_ARRAY_H16(VAL) ESR_CHECK_ARG(::estd::slice<const uint16_t>, VAL)
#endif // L_ARRAY_H16

#ifndef L_ARRAY_I32
#define L_ARRAY_I32(VAL) ESR_CHECK_ARG(::estd::slice<const int32_t>, VAL)
#endif // L_ARRAY_I32

#ifndef L_ARRAY_U32
#define L_ARRAY_U32(VAL) ESR_CHECK_ARG(::estd::slice<const uint32_t>, VAL)
#endif // L_ARRAY_U32

#ifndef L_ARRAY_H32
#define L_ARRAY_H32(VAL) ESR_CHECK_ARG(::estd::slice<const uint32_t>, VAL)
#endif // L_ARRAY_H32

#ifndef L_ARRAY_I64
#define L_ARRAY_I64(VAL) ESR_CHECK_ARG(::estd::slice<const int64_t>, VAL)
#endif // L_ARRAY_I64

#ifndef L_ARRAY_U64
#define L_ARRAY_U64(VAL) ESR_CHECK_ARG(::estd::slice<const uint64_t>, VAL)
#endif // L_ARRAY_U64

#ifndef L_ARRAY_H64
#define L_ARRAY_H64(VAL) ESR_CHECK_ARG(::estd::slice<const uint64_t>, VAL)
#endif // L_ARRAY_H64

#ifndef L_NAME
#define L_NAME(NAME, ARG)            \
    ESR_CHECK_ARG(char const*, NAME) \
    ESR_EVAL(ARG)
#endif // L_NAME

#ifndef L_UNIT
#define L_UNIT(UNIT, ARG) ESR_CHECK_ARG(char const*, UNIT) ESR_EVAL(ARG)
#endif // L_UNIT

#ifndef L_FMT
#define L_FMT(FORMAT, ...)             \
    ESR_CHECK_ARG(char const*, FORMAT) \
    ESR_FOR_EACH(ESR_EVAL, __VA_ARGS__)
#endif // L_FMT

#ifndef ESR_CHECK_ARG
namespace util
{
namespace logger
{

template<class T>
inline void esr_check_arg_type(T const& /*arg*/)
{}
} // namespace logger
} // namespace util

#define ESR_CHECK_ARG(TYPE, ARG) ::util::logger::esr_check_arg_type<TYPE>(static_cast<TYPE>(ARG));
#endif // ESR_CHECK_ARG

#ifndef ESR_CHECK_LOG
#define ESR_CHECK_LOG(COMPONENT, MESSAGE, ...) \
    do                                         \
    {                                          \
        ESR_CHECK_ARG(uint8_t, COMPONENT)      \
        ESR_CHECK_ARG(char const*, MESSAGE)    \
        ESR_FOR_EACH(ESR_EVAL, __VA_ARGS__)    \
    } while (false);
#endif // ESR_CHECK_LOG

#ifndef ESR_CHECK_ARG
#define ESR_CHECK_ARG(TYPE, ARG)
#endif // ESR_CHECK_ARG

#ifndef ESR_CHECK_LOG
#define ESR_CHECK_LOG(COMPONENT, MESSAGE, ...)
#endif // ESR_CHECK_LOG

#endif /* GUARD_77C21493_4E81_4360_B34E_B7960EA12621 */
