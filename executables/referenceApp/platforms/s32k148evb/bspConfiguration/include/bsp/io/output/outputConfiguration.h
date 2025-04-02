// Copyright 2024 Accenture.

// IGNORE_INCLUDE_GUARD_CHECK

#if defined(BSPOUTPUTCONFIG) && (BSPOUTPUTCONFIG == 1)

Output::OutputConfig const Output::sfOutputConfigurations[1][Output::NUMBER_OF_INTERNAL_OUTPUTS]
    = {{
        /*  0 */ {Io::EVAL_DO_1, Io::LOW, Io::HIGH_ACTIVE},
        /*  1 */ {Io::EVAL_LED_RED, Io::HIGH, Io::HIGH_ACTIVE},
    }};

Output::OutputConfig const* Output::getConfiguration(uint8_t /* hardwareVersion */)
{
    return &sfOutputConfigurations[0][0];
}

#else

enum OutputId
{
    /*  0 */ EVAL_DO_1,
    /*  1 */ EVAL_LED_RED,

    /* xx */ NUMBER_OF_INTERNAL_OUTPUTS,

    // TODO: dynamic outputs here
    /* yy */ // MyDynamicOutput,

    END_EXTERNAL_OUTPUTS,
    NUMBER_OF_EXTERNAL_OUTPUTS = END_EXTERNAL_OUTPUTS - NUMBER_OF_INTERNAL_OUTPUTS,
    TOTAL_NUMBER_OF_OUTPUTS    = NUMBER_OF_INTERNAL_OUTPUTS + NUMBER_OF_EXTERNAL_OUTPUTS,
    PORT_UNAVAILABLE           = TOTAL_NUMBER_OF_OUTPUTS
};

#endif /* BSP_OUTPUT_PIN_CONFIGURATION == 1 */
