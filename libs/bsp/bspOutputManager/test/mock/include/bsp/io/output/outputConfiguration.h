

#if (BSPOUTPUTCONFIG == 1)

Output::OutputConfig const Output::sfOutputConfigurations[][Output::NUMBER_OF_INTERNAL_OUTPUTS] = {
    {
        /* 00 */ {Io::MOCK_OUTPUT_1, Io::HIGH, Io::HIGH_ACTIVE},
    },
};

Output::OutputConfig const* Output::getConfiguration(uint8_t hw)
{
    return &sfOutputConfigurations[0][0];
}

#else

enum OutputId
{
    /* 00 */ MOCK_OUTPUT_1,
    NUMBER_OF_INTERNAL_OUTPUTS,
    NUMBER_OF_EXTERNAL_OUTPUTS = NUMBER_OF_INTERNAL_OUTPUTS - 1,
    TOTAL_NUMBER_OF_OUTPUTS,
};

#endif /* BSPOUTPUTCONFIG == 1 */
