// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "io/DynamicClientCfg.h"
#include "io/Io.h"
#include "platform/estdint.h"

namespace bios
{
class DigitalInput
{
    UNCOPYABLE(DigitalInput);

public:
#ifdef BSP_INPUT_PIN_CONFIGURATION
#undef BSP_INPUT_PIN_CONFIGURATION
#endif
#include "bsp/io/input/inputConfiguration.h"

    // Api for all DynamicClients
    class IDynamicInputClient
    {
        IDynamicInputClient& operator=(IDynamicInputClient const&);

    public:
        virtual bsp::BspReturnCode get(uint16_t chan, bool& result) = 0;
    };

    struct InputConfiguration
    {
        Io::PinId ioNumber;
        bool isInverted;
        uint8_t debounceThreshold;
    };

    struct DebounceConfiguration
    {
        uint8_t counter : 7;
        uint8_t vol     : 1;
    };

    DigitalInput() {}

    void init(uint8_t hw, bool doSetup = true);

    static void shutdown();

    void cyclic();

    /**
     * retrieves the current state of a specified digital input channel.
     */
    static bsp::BspReturnCode get(DigitalInputId channel, bool& result);

    /**
     * setDynamicInputClient
     * \param inputNumber see inputConfiguration
     * \param clientInputNumber number inside from OutputClient
     * \param see IDynamicInputClient
     * \return see bsp.h
     */

    static bsp::BspReturnCode
    setDynamicClient(uint16_t inputNumber, uint16_t clientInputNumber, IDynamicInputClient* client);

    /**
     * clrDynamicINputClient
     * \param inputNumber see inputConfiguration
     * \param see IDynamicOutputClient
     * \return see bsp.h
     */
    static bsp::BspReturnCode clrDynamicClient(uint16_t inputNumber);

    /**
     * \return the name of a digital input channel
     */
    static char const* getName(uint16_t channel);

private:
    InputConfiguration const* getConfiguration(uint8_t hw);

    static void cleanDynamicClients();

    static InputConfiguration const sfDigitalInputConfigurations[]
                                                                [NUMBER_OF_INTERNAL_DIGITAL_INPUTS];
    static InputConfiguration const* sfpDigitalInputConfiguration;

    using dynamicClientType = uint16_t;

    static uint8_t const InputAnzahlDynamic
        = ((NUMBER_OF_EXTERNAL_DIGITAL_INPUTS > 0)
               ? static_cast<uint8_t>(NUMBER_OF_EXTERNAL_DIGITAL_INPUTS)
               : 1U);

    static dynamicClient<dynamicClientType, IDynamicInputClient, 4, InputAnzahlDynamic>
        dynamicInputCfg;
#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
    static DebounceConfiguration debounced[NUMBER_OF_DIGITAL_INPUTS];
#endif
};

} /* namespace bios */
