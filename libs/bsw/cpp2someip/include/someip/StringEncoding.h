/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

namespace someip
{
enum class Encoding : uint8_t
{
    SOMEIP_ENCODING_UNKNOWN,
    SOMEIP_ENCODING_UTF8,
    SOMEIP_ENCODING_UTF16LE,
    SOMEIP_ENCODING_UTF16BE,
    SOMEIP_ENCODING_UTF8_NO_ZERO,
    SOMEIP_ENCODING_NO_BOM_NO_ZERO,
    SOMEIP_ENCODING_NO_BOM
};

struct UTF8
{
    static uint32_t const BOM_LENGTH         = 3U;
    static uint32_t const TERMINATION_LENGTH = 1U;

    UTF8()                       = delete;
    UTF8(UTF8 const&)            = delete;
    UTF8& operator=(UTF8 const&) = delete;

    struct MinStringLength
    {
        static uint32_t const value = BOM_LENGTH + TERMINATION_LENGTH;
    };

    template<uint32_t N>
    struct MaxStringLength
    {
        static uint32_t const value = (N - BOM_LENGTH) - TERMINATION_LENGTH;
    };

    static char const* BOM()
    {
        char const* const bom = "\xEF\xBB\xBF";
        return bom;
    }
};

struct UTF16LE
{
    static uint32_t const BOM_LENGTH         = 2U;
    static uint32_t const TERMINATION_LENGTH = 2U;

    UTF16LE()                          = delete;
    UTF16LE(UTF16LE const&)            = delete;
    UTF16LE& operator=(UTF16LE const&) = delete;

    struct MinStringLength
    {
        static uint32_t const value = BOM_LENGTH + TERMINATION_LENGTH;
    };

    template<uint32_t N>
    struct MaxStringLength
    {
        static uint32_t const value
            = (N - BOM_LENGTH)
              - (((N % 2U) == 1U) ? (TERMINATION_LENGTH + 1U) : TERMINATION_LENGTH);
    };

    static char const* BOM()
    {
        char const* const bom = "\xFF\xFE";
        return bom;
    }
};

struct UTF16BE
{
    static uint32_t const BOM_LENGTH         = 2U;
    static uint32_t const TERMINATION_LENGTH = 2U;

    UTF16BE()                          = delete;
    UTF16BE(UTF16BE const&)            = delete;
    UTF16BE& operator=(UTF16BE const&) = delete;

    struct MinStringLength
    {
        static uint32_t const value = BOM_LENGTH + TERMINATION_LENGTH;
    };

    template<uint32_t N>
    struct MaxStringLength
    {
        static uint32_t const value
            = (N - BOM_LENGTH)
              - (((N % 2U) == 1U) ? (TERMINATION_LENGTH + 1U) : TERMINATION_LENGTH);
    };

    static char const* BOM()
    {
        char const* const bom = "\xFE\xFF";
        return bom;
    }
};

struct UTF8NoZero
{
    static uint32_t const BOM_LENGTH         = 3U;
    static uint32_t const TERMINATION_LENGTH = 0U;

    UTF8NoZero()                             = delete;
    UTF8NoZero(UTF8NoZero const&)            = delete;
    UTF8NoZero& operator=(UTF8NoZero const&) = delete;

    struct MinStringLength
    {
        static uint32_t const value = BOM_LENGTH;
    };

    template<uint32_t N>
    struct MaxStringLength
    {
        static uint32_t const value = N - BOM_LENGTH;
    };

    static char const* BOM()
    {
        char const* const bom = "\xEF\xBB\xBF";
        return bom;
    }
};

struct NoBomNoZero
{
    static uint32_t const BOM_LENGTH         = 0U;
    static uint32_t const TERMINATION_LENGTH = 0U;

    NoBomNoZero()                              = delete;
    NoBomNoZero(NoBomNoZero const&)            = delete;
    NoBomNoZero& operator=(NoBomNoZero const&) = delete;

    struct MinStringLength
    {
        static uint32_t const value = 0U;
    };

    template<uint32_t N>
    struct MaxStringLength
    {
        static uint32_t const value = N;
    };

    static char const* BOM()
    {
        char const* const bom = "";
        return bom;
    }
};

struct NoBom
{
    static uint32_t const BOM_LENGTH         = 0U;
    static uint32_t const TERMINATION_LENGTH = 1U;

    NoBom()                        = delete;
    NoBom(NoBom const&)            = delete;
    NoBom& operator=(NoBom const&) = delete;

    struct MinStringLength
    {
        static uint32_t const value = TERMINATION_LENGTH;
    };

    template<uint32_t N>
    struct MaxStringLength
    {
        static uint32_t const value = N - TERMINATION_LENGTH;
    };

    static char const* BOM()
    {
        char const* const bom = "";
        return bom;
    }
};

} // namespace someip

namespace someip
{
namespace internal
{
struct EncodingHelper
{
    EncodingHelper()                                 = delete;
    EncodingHelper(EncodingHelper const&)            = delete;
    EncodingHelper& operator=(EncodingHelper const&) = delete;

    static uint32_t encodingLength(Encoding const e)
    {
        switch (e)
        {
            case Encoding::SOMEIP_ENCODING_UTF8:
            {
                return UTF8::BOM_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_UTF16LE:
            {
                return UTF16LE::BOM_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_UTF16BE:
            {
                return UTF16BE::BOM_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_UTF8_NO_ZERO:
            {
                return UTF8NoZero::BOM_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_NO_BOM_NO_ZERO:
            {
                return NoBomNoZero::BOM_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_NO_BOM:
            {
                return NoBom::BOM_LENGTH;
            }
            default:
            {
                return 0U;
            }
        }
    }

    static uint32_t terminationLength(Encoding const e)
    {
        switch (e)
        {
            case Encoding::SOMEIP_ENCODING_UTF8:
            {
                return UTF8::TERMINATION_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_UTF16LE:
            {
                return UTF16LE::TERMINATION_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_UTF16BE:
            {
                return UTF16BE::TERMINATION_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_UTF8_NO_ZERO:
            {
                return UTF8NoZero::TERMINATION_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_NO_BOM_NO_ZERO:
            {
                return NoBomNoZero::TERMINATION_LENGTH;
            }
            case Encoding::SOMEIP_ENCODING_NO_BOM:
            {
                return NoBom::TERMINATION_LENGTH;
            }
            default:
            {
                return 0U;
            }
        }
    }

    static char const* bom(Encoding const e)
    {
        switch (e)
        {
            case Encoding::SOMEIP_ENCODING_UTF8:
            {
                return UTF8::BOM();
            }
            case Encoding::SOMEIP_ENCODING_UTF16LE:
            {
                return UTF16LE::BOM();
            }
            case Encoding::SOMEIP_ENCODING_UTF16BE:
            {
                return UTF16BE::BOM();
            }
            case Encoding::SOMEIP_ENCODING_UTF8_NO_ZERO:
            {
                return UTF8NoZero::BOM();
            }
            case Encoding::SOMEIP_ENCODING_NO_BOM_NO_ZERO:
            {
                return NoBomNoZero::BOM();
            }
            case Encoding::SOMEIP_ENCODING_NO_BOM:
            {
                return NoBom::BOM();
            }
            default:
            {
                return nullptr;
            }
        }
    }
};

} // namespace internal
} // namespace someip
