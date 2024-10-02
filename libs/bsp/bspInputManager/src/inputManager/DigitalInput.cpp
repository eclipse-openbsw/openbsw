// Copyright 2024 Accenture.

#include "inputManager/DigitalInput.h"

#include "interrupts/SuspendResumeAllInterruptsLock.h"

#ifdef BSP_INPUT_PIN_CONFIGURATION
#undef BSP_INPUT_PIN_CONFIGURATION
#endif
#define BSP_INPUT_PIN_CONFIGURATION 1

namespace bios
{
#include "bsp/io/input/inputConfiguration.h"
#include "bsp/io/input/inputConfigurationStrings.h"
#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
DigitalInput::DebounceConfig DigitalInput::debounced[NUMBER_OF_DIGITAL_INPUTS];
#endif

DigitalInput::InputConfiguration const* DigitalInput::sfpDigitalInputConfiguration = nullptr;
dynamicClient<
    DigitalInput::dynamicClientType,
    DigitalInput::IDynamicInputClient,
    4,
    DigitalInput::InputAnzahlDynamic>
    DigitalInput::dynamicInputCfg;

void DigitalInput::init(uint8_t const hw, bool const doSetup)
{
    sfpDigitalInputConfiguration = getConfiguration(hw);
    if (nullptr == sfpDigitalInputConfiguration)
    {
        return;
    }
    if (true == doSetup)
    {
        for (int32_t i = 0; i < NUMBER_OF_INTERNAL_DIGITAL_INPUTS; i++)
        {
            (void)Io::setDefaultConfiguration(
                static_cast<uint16_t>(sfpDigitalInputConfiguration[i].ioNumber));
        }
    }
    cleanDynamicClients();
    cyclic();
}

void DigitalInput::cyclic(void)
{
    if (nullptr == sfpDigitalInputConfiguration)
    {
        return;
    }

#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
    bool vol;
    for (uint16_t i = 0; i < NUMBER_OF_INTERNAL_DIGITAL_INPUTS; i++)
    {
        vol = Io::getPin(i);

        if (vol)
        {
            if (debounced[i].counter < digitalInputs[i].debounceThreshold)
            {
                debounced[i].counter++;
            }
            else
            {
                debounced[i].vol = 1;
            }
        }
        else
        {
            if (debounced[i].counter > 0)
            {
                debounced[i].counter--;
            }
            else
            {
                debounced[i].vol = 0;
            }
        }
    }
#endif
}

void DigitalInput::shutdown(void) { cleanDynamicClients(); }

bsp::BspReturnCode DigitalInput::get(DigitalInputId const channel, bool& result)
{
    if (nullptr == sfpDigitalInputConfiguration)
    {
        return bsp::BSP_ERROR;
    }

    if (channel < NUMBER_OF_INTERNAL_DIGITAL_INPUTS)
    {
        if (Io::PORT_UNAVAILABLE == sfpDigitalInputConfiguration[channel].ioNumber)
        {
            return bsp::BSP_NOT_SUPPORTED;
        }
#if (INPUTDIGITAL_DEBOUNCE_ACTIVE == 1)
        result = debounced[channel].vol != 0U;
#else
        result = Io::getPin(static_cast<uint16_t>(sfpDigitalInputConfiguration[channel].ioNumber));
#endif

        if (sfpDigitalInputConfiguration[channel].isInverted)
        {
            result = !result;
        }
        return bsp::BSP_OK;
    }
    else if (channel == NUMBER_OF_INTERNAL_DIGITAL_INPUTS)
    {
        return bsp::BSP_ERROR;
    }
    else if (channel < TOTAL_NUMBER_OF_DIGITAL_INPUTS)
    {
        // dynamic instance
        dynamicClientType const dynamicChannel
            = static_cast<dynamicClientType>(channel - NUMBER_OF_INTERNAL_DIGITAL_INPUTS - 1);
        if (dynamicChannel < InputAnzahlDynamic)
        {
            if (dynamicInputCfg.getClientValid(dynamicChannel) == true)
            {
                return (dynamicInputCfg.getClientInstance(dynamicChannel)
                            ->get(dynamicInputCfg.getChannelInsideClient(dynamicChannel), result));
            }
            else
            {
                return bsp::BSP_ERROR;
            }
        }
        else
        {
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode DigitalInput::setDynamicClient(
    uint16_t const inputNumber, uint16_t const clientInputNumber, IDynamicInputClient* const client)
{
    // range of outputNumber check
    if ((inputNumber > static_cast<uint16_t>(NUMBER_OF_INTERNAL_DIGITAL_INPUTS))
        && (inputNumber < static_cast<uint16_t>(TOTAL_NUMBER_OF_DIGITAL_INPUTS)))
    {
        dynamicClientType const dynamicChannel
            = inputNumber - static_cast<uint16_t>(NUMBER_OF_INTERNAL_DIGITAL_INPUTS) - 1U;
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicInputCfg.setDynamicClient(dynamicChannel, clientInputNumber, client) == true)
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        else
        {
            fLock.resume();
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

bsp::BspReturnCode DigitalInput::clrDynamicClient(uint16_t const inputNumber)
{
    if ((inputNumber > static_cast<uint16_t>(NUMBER_OF_INTERNAL_DIGITAL_INPUTS))
        && (inputNumber < static_cast<uint16_t>(TOTAL_NUMBER_OF_DIGITAL_INPUTS)))
    {
        dynamicClientType const dynamicChannel
            = inputNumber - static_cast<uint16_t>(NUMBER_OF_INTERNAL_DIGITAL_INPUTS) - 1U;
        interrupts::SuspendResumeAllInterruptsLock fLock;
        fLock.suspend();
        if (dynamicInputCfg.clearDynamicClient(dynamicChannel) == true)
        {
            fLock.resume();
            return bsp::BSP_OK;
        }
        else
        {
            fLock.resume();
            return bsp::BSP_ERROR;
        }
    }
    else
    {
        return bsp::BSP_ERROR;
    }
}

void DigitalInput::cleanDynamicClients()
{
    interrupts::SuspendResumeAllInterruptsLock fLock;
    fLock.suspend();
    dynamicInputCfg.cleanDynamicClients();
    fLock.resume();
}

char const* DigitalInput::getName(uint16_t const channel)
{
    if (channel < ((sizeof(inputConfigurationStrings)) / (sizeof(char*))))
    {
        return inputConfigurationStrings[channel];
    }
    else
    {
        return ("??? ->");
    }
}

} /* namespace bios */
