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

#if defined(BSP_INPUT_PIN_CONFIGURATION) && (BSP_INPUT_PIN_CONFIGURATION == 1)
DigitalInput::InputConfiguration const
    DigitalInput::sfDigitalInputConfigurations[][DigitalInput::NUMBER_OF_INTERNAL_DIGITAL_INPUTS]
    = {
        {},
};

DigitalInput::InputConfiguration const*
DigitalInput::getConfiguration(uint8_t /* hardwareVersion */)
{
    return static_cast<DigitalInput::InputConfiguration const*>(
        nullptr); //&sfDigitalInputConfigurations[0][0];
}

#else

enum DigitalInputId
{
    LAST_INTERNAL_DIGITAL_INPUT = -1, // EVAL_SW3,
    // TODO: dynamic inputs go here
    //       update LAST_DYNAMIC_DIGITAL_INPUT when adding a new external input
    /* yy */ // MyFirstDynamicInput,
    LAST_DYNAMIC_DIGITAL_INPUT  = LAST_INTERNAL_DIGITAL_INPUT,
    PORT_UNAVAILABLE
};
#endif /* #if (BSP_INPUT_PIN_CONFIGURATION == 1) */
