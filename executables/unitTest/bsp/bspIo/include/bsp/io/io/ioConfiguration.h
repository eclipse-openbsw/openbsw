#if (BSP_IO_PIN_CONFIGURATION == 1)

#define MODULE_P22 /*lint --e(923, 9078)*/ ((*(Ifx_P*)0xF003B600u))

const Io::PinConfiguration Io::fPinConfiguration[Io::NUMBER_OF_IOS] = {

    /* 00 */ { 0UL },

};

#else

enum PinId
{
    /* 00 */ TestPin0,

    /* 00 */ NUMBER_OF_INPUTS_AND_OUTPUTS,
    /* 00 */ PORT_UNAVAILABLE = NUMBER_OF_INPUTS_AND_OUTPUTS

};

#endif /* BSP_IO_PIN_CONFIGURATION == 1 */