/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#if (BSP_IO_PIN_CONFIGURATION == 1)

#define MODULE_P22 /*lint --e(923, 9078)*/ ((*(Ifx_P*)0xF003B600u))

Io::PinConfiguration const Io::fPinConfiguration[Io::NUMBER_OF_IOS] = {

    /* 00 */ {0UL},

};

#else

enum PinId
{
    /* 00 */ TestPin0,

    /* 00 */ NUMBER_OF_INPUTS_AND_OUTPUTS,
    /* 00 */ PORT_UNAVAILABLE = NUMBER_OF_INPUTS_AND_OUTPUTS

};

#endif /* BSP_IO_PIN_CONFIGURATION == 1 */
