#if (BSP_INPUT_PIN_CONFIGURATION == 1)

DigitalInput::InputConfiguration const
    DigitalInput::sfDigitalInputConfigurations[][DigitalInput::NUMBER_OF_INTERNAL_DIGITAL_INPUTS]
    = {
        {/*  0 */ {Io::TestPin0, Io::HIGH_ACTIVE, 1}},
};

DigitalInput::InputConfiguration const*
DigitalInput::getConfiguration(uint8_t /* hardwareVersion */)
{
    return &sfDigitalInputConfigurations[0][0];
}

#else

enum DigitalInputId
{
    InternalInput1,
    InternalInput2,
    LAST_INTERNAL_DIGITAL_INPUT = InternalInput2,
    ExternalInput1,
    ExternalInput2,
    LAST_DYNAMIC_DIGITAL_INPUT = ExternalInput2,
    PORT_UNAVAILABLE
};

#endif /* BSP_INPUT_PIN_CONFIGURATION == 1 */
