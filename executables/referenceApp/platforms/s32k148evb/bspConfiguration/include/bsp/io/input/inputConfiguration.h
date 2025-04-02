// Copyright 2024 Accenture.

// IGNORE_INCLUDE_GUARD_CHECK

#if defined(BSP_INPUT_PIN_CONFIGURATION) && (BSP_INPUT_PIN_CONFIGURATION == 1)
DigitalInput::InputConfiguration const
    DigitalInput::sfDigitalInputConfigurations[][DigitalInput::NUMBER_OF_INTERNAL_DIGITAL_INPUTS]
    = {
        {
            /*  0 */ {Io::EVAL_DI_1, Io::HIGH_ACTIVE, 1},
            /*  1 */ {Io::EVAL_SW3, Io::HIGH_ACTIVE, 1},
        },
};

DigitalInput::InputConfiguration const*
DigitalInput::getConfiguration(uint8_t /* hardwareVersion */)
{
    return &sfDigitalInputConfigurations[0][0];
}

#else

enum DigitalInputId
{
    /*  0 */ EVAL_DI_1,
    /* 1 */ EVAL_SW3,

    /* xx */ NUMBER_OF_INTERNAL_DIGITAL_INPUTS,

    // TODO: dynamic inputs here
    /* yy */ // MyDynamicInput,

    NUMBER_OF_EXTERNAL_DIGITAL_INPUTS,
    TOTAL_NUMBER_OF_DIGITAL_INPUTS = NUMBER_OF_EXTERNAL_DIGITAL_INPUTS,
    PORT_UNAVAILABLE               = TOTAL_NUMBER_OF_DIGITAL_INPUTS
};
#endif /* #if (BSP_INPUT_PIN_CONFIGURATION == 1) */
