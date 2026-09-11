/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// IGNORE_INCLUDE_GUARD_CHECK

#if defined(BSP_IO_PIN_CONFIGURATION) && (BSP_IO_PIN_CONFIGURATION == 1)

Io::PinConfiguration const Io::fPinConfiguration[Io::NUMBER_OF_IOS] = {
// clang-format off

#if 0
    /* 00  */ {_PORTA_, PA0, _IN, (uint8_t) FILTER_ACTIVE | (uint8_t) FILTER_TICK1, GPIO},
    /* 01  */ {_PORTA_, PA1, _OUT, (uint8_t) FILTER_ACTIVE | (uint8_t) FILTER_TICK1, (uint32_t) GPIO | (uint32_t) STRENGTH_ON},
    /* 02  */ {_PORTA_, PA2, _IN, (uint8_t) FILTER_ACTIVE | (uint8_t) FILTER_TICK1, PINDISABLE},
    /* 03  */ {_PORTA_, PA3, _IN, (uint8_t) FILTER_ACTIVE | (uint8_t) FILTER_TICK1, PINDISABLE},
#endif
    // clang-format on
};

#else

enum PinId
{
    PIN_GP0,
    PIN_GP1,
    PIN_GP2,
    PIN_GP3,
    PIN_GP4,
    PIN_GP5,
    PIN_GP6,
    PIN_GP7,
    PIN_GP8,
    PIN_GP9,
    PIN_GP10,
    PIN_GP11,
    PIN_GP12,
    PIN_GP13,
    PIN_GP14,
    PIN_GP15,
    PIN_GP16,
    PIN_GP17,
    PIN_GP18,
    PIN_GP19,
    PIN_GP20,
    PIN_GP21,
    PIN_GP22,
    /* 0 */ EVAL_LED_RED, // PIN_GP25
    PIN_GP26,
    PIN_GP27,
    PIN_GP28,

    // The Pi Pico 2 has a single onboard LED and no dedicated safety test pin. These IDs exist so
    // that the generic safety console command can be built, and are expected to be wired to
    // external hardware on GP14/GP15.
    /* 1 */ EVAL_LED_GREEN, // PIN_GP14
    /* 2 */ SAFETY_TEST,    // PIN_GP15

    /* xx */ NUMBER_OF_INPUTS_AND_OUTPUTS,
    /* xx */ PORT_UNAVAILABLE = NUMBER_OF_INPUTS_AND_OUTPUTS,
};

#endif /* BSP_IO_PIN_CONFIGURATION == 1 */
